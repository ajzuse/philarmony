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
 * DisplayManager - Implementation (DriverRegistry-backed display instances)
 */
#include "DisplayManager.hpp"
#include "../../core/DriverRegistry.hpp"

namespace filament_dryer {

namespace {
constexpr const char* kDetectOrder[] = {
    "st7789", "ili9341", "ssd1306", "sh1106",
    "st7735", "gc9a01", "ili9488", "hd44780", "nextion"
};
constexpr size_t kDetectCount = sizeof(kDetectOrder) / sizeof(kDetectOrder[0]);
}  // namespace

DisplayManager::DisplayManager() {}

DisplayManager::~DisplayManager() {
    end();
}

bool DisplayManager::begin(const JsonObject& config) {
    end();
    String driver = config["driver"] | "auto";

    if (driver == "auto") {
        for (size_t i = 0; i < kDetectCount; ++i) {
            if (tryDriver(kDetectOrder[i], config)) {
                break;
            }
        }
    } else {
        tryDriver(driver, config);
    }

    if (active_display_) {
        autoConfigureLayout(active_display_->getMetrics());

        if (config.containsKey("layout")) {
            setLayout(config["layout"].as<JsonObject>());
        }

        layout_.refresh_rate_hz =
            constrain((uint8_t)(config["refresh_rate_hz"] | 1), 1, 5);
    }

    return active_display_ != nullptr;
}

void DisplayManager::end() {
    if (owned_display_) {
        owned_display_->clear();
        delete owned_display_;
        owned_display_ = nullptr;
    }
    active_display_ = nullptr;
    active_type_ = "none";
    last_refresh_ms_ = 0;
}

bool DisplayManager::setDisplayType(const String& type, const JsonObject& config) {
    end();
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

    if (!isRefreshDue()) return;

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

bool DisplayManager::tryDriver(const String& type, const JsonObject& config) {
    // Factory already calls begin(); fail closed if create/begin fails.
    IDisplayDriver* drv = DriverRegistry::instance().createDisplay(type, config);
    if (!drv) return false;

    if (!drv->isConnected()) {
        delete drv;
        return false;
    }

    if (owned_display_) {
        delete owned_display_;
        owned_display_ = nullptr;
    }
    owned_display_  = drv;
    active_display_ = drv;
    active_type_    = type;
    return true;
}

void DisplayManager::autoConfigureLayout(const DisplayMetrics& m) {
    layout_.fields.clear();

    const std::vector<String> all_fields = {
        "chamber_temp_c", "target_temp_c", "humidity_pct",
        "target_humidity_pct", "heater_power_pct", "exhaust_fan_power_pct",
        "heater_on", "exhaust_fan_on", "status",
        "elapsed_time_sec", "remaining_time_sec", "uptime_sec",
        "cpu_usage_pct", "memory_free_bytes"
    };

    if (active_type_ == "hd44780") {
        if (m.height >= 4) {
            layout_.fields = {"chamber_temp_c", "target_temp_c",
                               "humidity_pct", "heater_power_pct",
                               "heater_on", "status"};
        } else {
            layout_.fields = {"chamber_temp_c", "heater_power_pct", "status"};
        }
        layout_.font_scaling = FontScaling::NORMAL;
        layout_.compact_mode = (m.height <= 2);
        return;
    }

    uint32_t pixel_count = (uint32_t)m.width * m.height;

    if (pixel_count <= 128u * 32u) {
        layout_.fields       = {"chamber_temp_c", "heater_power_pct", "status"};
        layout_.font_scaling = FontScaling::TINY;
        layout_.compact_mode = true;
    } else if (pixel_count <= 128u * 64u) {
        layout_.fields       = {"chamber_temp_c", "target_temp_c",
                                 "humidity_pct",   "heater_power_pct",
                                 "heater_on",      "status",
                                 "elapsed_time_sec"};
        layout_.font_scaling = FontScaling::SMALL;
        layout_.compact_mode = false;
    } else if (pixel_count <= 135u * 240u) {
        layout_.fields       = {"chamber_temp_c", "target_temp_c",
                                 "humidity_pct",   "heater_power_pct",
                                 "exhaust_fan_power_pct", "heater_on",
                                 "exhaust_fan_on", "status",
                                 "elapsed_time_sec", "uptime_sec"};
        layout_.font_scaling = FontScaling::SMALL;
        layout_.compact_mode = false;
    } else if (pixel_count <= 240u * 240u) {
        layout_.fields       = {"chamber_temp_c", "target_temp_c",
                                 "humidity_pct",   "heater_power_pct",
                                 "heater_on", "exhaust_fan_on", "status"};
        layout_.font_scaling = FontScaling::NORMAL;
        layout_.compact_mode = false;
    } else if (pixel_count <= 240u * 320u) {
        layout_.fields       = all_fields;
        layout_.font_scaling = FontScaling::NORMAL;
        layout_.compact_mode = false;
    } else {
        layout_.fields       = all_fields;
        layout_.font_scaling = FontScaling::LARGE;
        layout_.compact_mode = false;
    }

    if (pixel_count <= 128u * 64u) {
        layout_.refresh_rate_hz = 2;
    } else {
        layout_.refresh_rate_hz = 1;
    }
}

void DisplayManager::buildRenderPayload(const JsonObject& src,
                                         JsonDocument& out) const {
    out.clear();
    JsonObject dst = out.to<JsonObject>();

    if (layout_.fields.empty()) {
        for (JsonPair kv : src) {
            dst[kv.key()] = kv.value();
        }
        dst["font_scaling"] = static_cast<int>(layout_.font_scaling);
        dst["compact_mode"] = layout_.compact_mode;
        return;
    }

    for (const String& field : layout_.fields) {
        if (src.containsKey(field)) {
            dst[field] = src[field];
        }
    }

    dst["font_scaling"] = static_cast<int>(layout_.font_scaling);
    dst["compact_mode"] = layout_.compact_mode;

    if (src.containsKey("status") && !dst.containsKey("status")) {
        dst["status"] = src["status"];
    }
}

} // namespace filament_dryer
