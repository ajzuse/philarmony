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
 * DisplayManager - Auto-detect, manage, and render via DriverRegistry factories.
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class DisplayManager {
public:
    enum class FontScaling { AUTO, TINY, SMALL, NORMAL, LARGE };

    struct LayoutConfig {
        std::vector<String> fields;
        FontScaling font_scaling = FontScaling::AUTO;
        bool compact_mode        = false;
        bool show_graph          = false;
        uint8_t refresh_rate_hz  = 1;
    };

    DisplayManager();
    ~DisplayManager();

    bool begin(const JsonObject& config);
    void end();
    bool setDisplayType(const String& type, const JsonObject& config);
    void setLayout(const JsonObject& layout_config);
    void update(const JsonObject& status_fields);
    void clear();
    void showError(const String& message);
    void showBootScreen(const String& firmware_version);
    IDisplayDriver* getActiveDriver() { return active_display_; }
    const IDisplayDriver* getActiveDriver() const { return active_display_; }
    void pushRgb565(int16_t x, int16_t y, uint16_t w, uint16_t h,
                    const uint16_t* data);
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const;

    bool         isAnyConnected()  const;
    String       getActiveType()   const { return active_type_; }
    LayoutConfig getLayout()       const { return layout_; }

    bool isRefreshDue() const;
    void markRefreshed();
    String getLayoutPreview() const;

private:
    IDisplayDriver* owned_display_ = nullptr;
    IDisplayDriver* active_display_ = nullptr;
    String          active_type_    = "none";
    LayoutConfig    layout_;
    uint32_t        last_refresh_ms_ = 0;

    bool tryDriver(const String& type, const JsonObject& config);
    void autoConfigureLayout(const DisplayMetrics& metrics);
    void buildRenderPayload(const JsonObject& src, JsonDocument& out) const;
};

} // namespace filament_dryer
