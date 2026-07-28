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
 * DisplayManager - Auto-detect, manage, and render on any supported display driver.
 *
 * T028g: Auto-detection tries ST7789 → ILI9341 → SSD1306 in order.
 * T028h: Auto-layout engine adapts font size, field density, and compact mode
 *        based on driver resolution.
 * T028i: Configurable display refresh rate (1 Hz – 5 Hz), decoupled from the
 *        1 Hz WebSocket status stream.
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "../interfaces/IDriverInterfaces.hpp"

// All supported display drivers
#include "SSD1306Display.hpp"
#include "SH1106Display.hpp"
#include "ST7789Display.hpp"
#include "ST7735Display.hpp"
#include "ILI9341Display.hpp"
#include "GC9A01Display.hpp"
#include "ILI9488Display.hpp"
#include "HD44780Display.hpp"
#include "NextionDisplay.hpp"

namespace filament_dryer {

class DisplayManager {
public:
    // ---- Auto-layout engine types ----
    enum class FontScaling { AUTO, TINY, SMALL, NORMAL, LARGE };

    struct LayoutConfig {
        std::vector<String> fields;           // ordered list of status fields to show
        FontScaling font_scaling = FontScaling::AUTO;
        bool compact_mode        = false;
        bool show_graph          = false;
        uint8_t refresh_rate_hz  = 1;         // 1–5 Hz display refresh
    };

    // ---- Public API ----
    DisplayManager();
    ~DisplayManager();

    /**
     * begin() - initialise from a full hardware config JSON object.
     * The "display" key inside config is consumed.
     * If driver == "auto", auto-detection is attempted.
     */
    bool begin(const JsonObject& config);

    /** Tear down the active display driver so hot-reload can re-init cleanly. */
    void end();

    /**
     * setDisplayType() - manually select and initialise a specific driver.
     * Useful when the caller already knows the display type from NVS config.
     */
    bool setDisplayType(const String& type, const JsonObject& config);

    /** Apply an auto-layout configuration from a JSON object. */
    void setLayout(const JsonObject& layout_config);

    /** Call this at your desired refresh interval (e.g. every 200 ms for 5 Hz). */
    void update(const JsonObject& status_fields);

    void clear();
    void showError(const String& message);
    void showBootScreen(const String& firmware_version);

    bool         isAnyConnected()  const;
    String       getActiveType()   const { return active_type_; }
    LayoutConfig getLayout()       const { return layout_; }

    /** Returns true if enough time has elapsed since the last render. */
    bool isRefreshDue() const;

    /** Reset the refresh timer (call after each successful update()). */
    void markRefreshed();

    /** Human-readable layout summary for diagnostics. */
    String getLayoutPreview() const;

private:
    // Storage for every driver (statically allocated, no heap fragmentation)
    SSD1306Display  ssd1306_;
    SH1106Display   sh1106_;
    ST7789Display   st7789_;
    ST7735Display   st7735_;
    ILI9341Display  ili9341_;
    GC9A01Display   gc9a01_;
    ILI9488Display  ili9488_;
    HD44780Display  hd44780_;
    NextionDisplay  nextion_;

    IDisplayDriver* active_display_ = nullptr;
    String          active_type_    = "none";
    LayoutConfig    layout_;
    uint32_t        last_refresh_ms_ = 0;

    // T028g: auto-detect order
    static constexpr const char* kDetectOrder[] = {
        "st7789", "ili9341", "ssd1306", "sh1106",
        "st7735", "gc9a01", "ili9488", "hd44780", "nextion"
    };
    static constexpr size_t kDetectCount =
        sizeof(kDetectOrder) / sizeof(kDetectOrder[0]);

    bool tryDriver(const String& type, const JsonObject& config);
    IDisplayDriver* driverFor(const String& type);

    // T028h: derive sensible layout defaults from display metrics
    void autoConfigureLayout(const DisplayMetrics& metrics);

    // T028h: build the JsonObject the driver update() call actually receives,
    // applying field selection, and labelling compact vs full.
    void buildRenderPayload(const JsonObject& src, JsonDocument& out) const;
};

} // namespace filament_dryer
