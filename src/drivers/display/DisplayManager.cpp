/*
 * Philarmony Filament Dryer ESP32 Firmware
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * DisplayManager - Implementation
 *
 * T028g: Auto-detection of 9 display types.
 * T028h: Auto-layout engine — font scaling, field density, compact mode
 *        derived from screen resolution.
 * T028i: Configurable refresh rate (1 Hz – 5 Hz) decoupled from status stream.
 */
#include "DisplayManager.hpp"

namespace filament_dryer {

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

DisplayManager::DisplayManager() {}

DisplayManager::~DisplayManager() {}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool DisplayManager::begin(const JsonObject& config) {
    end();
    String driver = config["driver"] | "auto";

    if (driver == "auto") {
        // T028g: try every driver in detection order
        for (size_t i = 0; i < kDetectCount; ++i) {
            if (tryDriver(kDetectOrder[i], config)) {
                break;
            }
        }
    } else {
        tryDriver(driver, config);
    }

    if (active_display_) {
        // T028h: configure layout based on actual display metrics
        autoConfigureLayout(active_display_->getMetrics());

        // Apply user layout overrides if present
        if (config.containsKey("layout")) {
            setLayout(config["layout"].as<JsonObject>());
        }

        // T028i: refresh rate from config (default 1 Hz)
        layout_.refresh_rate_hz =
            constrain((uint8_t)(config["refresh_rate_hz"] | 1), 1, 5);
    }

    return active_display_ != nullptr;
}

void DisplayManager::end() {
    if (active_display_) {
        active_display_->clear();
        active_display_ = nullptr;
        active_type_ = "none";
    }
    last_refresh_ms_ = 0;
}

bool DisplayManager::setDisplayType(const String& type, const JsonObject& config) {
    // Deactivate current driver first
    if (active_display_) {
        active_display_->clear();
        active_display_ = nullptr;
        active_type_    = "none";
    }

    if (!tryDriver(type, config)) return false;

    autoConfigureLayout(active_display_->getMetrics());
    layout_.refresh_rate_hz =
        constrain((uint8_t)(config["refresh_rate_hz"] | 1), 1, 5);

    if (config.containsKey("layout")) {
        setLayout(config["layout"].as<JsonObject>());
    }
    return true;
}

void DisplayManager::setLayout(const JsonObject& layout_config) {
    if (layout_config.containsKey("fields")) {
        JsonArray arr = layout_config["fields"].as<JsonArray>();
        layout_.fields.clear();
        for (JsonVariant v : arr) {
            layout_.fields.push_back(v.as<String>());
        }
    }

    if (layout_config.containsKey("font_scaling")) {
        String fs = layout_config["font_scaling"].as<String>();
        if      (fs == "tiny")   layout_.font_scaling = FontScaling::TINY;
        else if (fs == "small")  layout_.font_scaling = FontScaling::SMALL;
        else if (fs == "medium" || fs == "normal") layout_.font_scaling = FontScaling::NORMAL;
        else if (fs == "large")  layout_.font_scaling = FontScaling::LARGE;
        else                     layout_.font_scaling = FontScaling::AUTO;
    }

    layout_.compact_mode    = layout_config["compact_mode"]   | layout_.compact_mode;
    layout_.show_graph      = layout_config["show_graph"]     | layout_.show_graph;
    layout_.refresh_rate_hz = constrain(
        (uint8_t)(layout_config["refresh_rate_hz"] | layout_.refresh_rate_hz), 1, 5);
}

void DisplayManager::update(const JsonObject& status_fields) {
    if (!active_display_ || !active_display_->isConnected()) return;

    // T028i: only render when the refresh interval has elapsed
    if (!isRefreshDue()) return;

    // T028h: build filtered / layout-adjusted payload
    JsonDocument render_doc;
    buildRenderPayload(status_fields, render_doc);

    active_display_->update(render_doc.as<JsonObject>());
    markRefreshed();
}

void DisplayManager::clear() {
    if (active_display_) active_display_->clear();
}

void DisplayManager::showError(const String& message) {
    if (active_display_) active_display_->showError(message);
}

void DisplayManager::showBootScreen(const String& firmware_version) {
    if (active_display_) active_display_->showBootScreen(firmware_version);
}

bool DisplayManager::isAnyConnected() const {
    return active_display_ && active_display_->isConnected();
}

// T028i: refresh-rate gate
bool DisplayManager::isRefreshDue() const {
    uint32_t interval_ms = 1000u / constrain(layout_.refresh_rate_hz, 1, 5);
    return (millis() - last_refresh_ms_) >= interval_ms;
}

void DisplayManager::markRefreshed() {
    last_refresh_ms_ = millis();
}

String DisplayManager::getLayoutPreview() const {
    String s = "Driver: " + active_type_ + "\n";
    s += "Refresh: " + String(layout_.refresh_rate_hz) + " Hz\n";
    s += "Compact: " + String(layout_.compact_mode ? "yes" : "no") + "\n";
    s += "Fields: ";
    for (size_t i = 0; i < layout_.fields.size(); ++i) {
        if (i) s += ", ";
        s += layout_.fields[i];
    }
    return s;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

IDisplayDriver* DisplayManager::driverFor(const String& type) {
    if (type == "ssd1306")  return &ssd1306_;
    if (type == "sh1106")   return &sh1106_;
    if (type == "st7789")   return &st7789_;
    if (type == "st7735")   return &st7735_;
    if (type == "ili9341")  return &ili9341_;
    if (type == "gc9a01")   return &gc9a01_;
    if (type == "ili9488")  return &ili9488_;
    if (type == "hd44780")  return &hd44780_;
    if (type == "nextion")  return &nextion_;
    return nullptr;
}

bool DisplayManager::tryDriver(const String& type, const JsonObject& config) {
    IDisplayDriver* drv = driverFor(type);
    if (!drv) return false;

    // Inject the driver key so each driver's begin() can read it
    // We work with the config as-is; each driver reads only its own keys
    if (drv->begin(config) && drv->isConnected()) {
        active_display_ = drv;
        active_type_    = type;
        return true;
    }
    return false;
}

// T028h: Auto-layout engine
void DisplayManager::autoConfigureLayout(const DisplayMetrics& m) {
    layout_.fields.clear();

    // Determine a baseline set of fields ordered by importance
    const std::vector<String> all_fields = {
        "chamber_temp_c", "target_temp_c", "humidity_pct",
        "target_humidity_pct", "heater_power_pct", "exhaust_fan_power_pct",
        "heater_on", "exhaust_fan_on", "status",
        "elapsed_time_sec", "remaining_time_sec", "uptime_sec",
        "cpu_usage_pct", "memory_free_bytes"
    };

    // ---- Character LCD (HD44780) ----
    if (active_type_ == "hd44780") {
        // metrics encode cols x rows in width x height
        if (m.height >= 4) {
            // 20x4: show 6 fields
            layout_.fields = {"chamber_temp_c", "target_temp_c",
                               "humidity_pct", "heater_power_pct",
                               "heater_on", "status"};
        } else {
            // 16x2: show 3 fields
            layout_.fields = {"chamber_temp_c", "heater_power_pct", "status"};
        }
        layout_.font_scaling = FontScaling::NORMAL;
        layout_.compact_mode = (m.height <= 2);
        return;
    }

    // ---- Pixel displays ----
    uint32_t pixel_count = (uint32_t)m.width * m.height;

    if (pixel_count <= 128u * 32u) {
        // Very small OLED (128x32)
        layout_.fields       = {"chamber_temp_c", "heater_power_pct", "status"};
        layout_.font_scaling = FontScaling::TINY;
        layout_.compact_mode = true;

    } else if (pixel_count <= 128u * 64u) {
        // Standard 128x64 OLED (SSD1306, SH1106)
        layout_.fields       = {"chamber_temp_c", "target_temp_c",
                                 "humidity_pct",   "heater_power_pct",
                                 "heater_on",      "status",
                                 "elapsed_time_sec"};
        layout_.font_scaling = FontScaling::SMALL;
        layout_.compact_mode = false;

    } else if (pixel_count <= 135u * 240u) {
        // Small TFT (ST7789 135x240, ST7735 128x160)
        layout_.fields       = {"chamber_temp_c", "target_temp_c",
                                 "humidity_pct",   "heater_power_pct",
                                 "exhaust_fan_power_pct", "heater_on",
                                 "exhaust_fan_on", "status",
                                 "elapsed_time_sec", "uptime_sec"};
        layout_.font_scaling = FontScaling::SMALL;
        layout_.compact_mode = false;

    } else if (pixel_count <= 240u * 240u) {
        // Round TFT (GC9A01 240x240)
        layout_.fields       = {"chamber_temp_c", "target_temp_c",
                                 "humidity_pct",   "heater_power_pct",
                                 "heater_on", "exhaust_fan_on", "status"};
        layout_.font_scaling = FontScaling::NORMAL;
        layout_.compact_mode = false;

    } else if (pixel_count <= 240u * 320u) {
        // Medium TFT (ILI9341 240x320, ST7789 240x240)
        layout_.fields       = all_fields; // show everything
        layout_.font_scaling = FontScaling::NORMAL;
        layout_.compact_mode = false;

    } else {
        // Large TFT (ILI9488 320x480 and above)
        layout_.fields       = all_fields;
        layout_.font_scaling = FontScaling::LARGE;
        layout_.compact_mode = false;
    }

    // Default refresh rate based on display size — smaller displays can go faster
    if (pixel_count <= 128u * 64u) {
        layout_.refresh_rate_hz = 2; // OLEDs: 2 Hz default
    } else {
        layout_.refresh_rate_hz = 1; // TFTs: 1 Hz default
    }
}

// T028h: field-selection filter — only passes fields present in layout_.fields
void DisplayManager::buildRenderPayload(const JsonObject& src,
                                         JsonDocument& out) const {
    out.clear();
    JsonObject dst = out.to<JsonObject>();

    if (layout_.fields.empty()) {
        // No filter: pass everything through
        for (JsonPair kv : src) {
            dst[kv.key()] = kv.value();
        }
        dst["font_scaling"] = static_cast<int>(layout_.font_scaling);
        dst["compact_mode"] = layout_.compact_mode;
        return;
    }

    // Pass only the fields explicitly listed in the layout
    for (const String& field : layout_.fields) {
        if (src.containsKey(field)) {
            dst[field] = src[field];
        }
    }

    // Always pass layout hints so drivers can size text
    dst["font_scaling"] = static_cast<int>(layout_.font_scaling);
    dst["compact_mode"] = layout_.compact_mode;

    // Always pass "status" — drivers need it for color coding even in compact mode
    if (src.containsKey("status") && !dst.containsKey("status")) {
        dst["status"] = src["status"];
    }
}

constexpr const char* DisplayManager::kDetectOrder[];

} // namespace filament_dryer
