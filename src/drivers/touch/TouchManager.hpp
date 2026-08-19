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
 * TouchManager - Touch controller probing, calibration, and filtering
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class TouchManager {
public:
    struct Calibration {
        int16_t x_min = 0;
        int16_t x_max = 0;
        int16_t y_min = 0;
        int16_t y_max = 0;
        uint16_t width = 0;
        uint16_t height = 0;
        bool swap_xy = false;
        bool invert_x = false;
        bool invert_y = false;

        bool isValid() const {
            return x_max > x_min && y_max > y_min && width > 0 && height > 0;
        }
    };

    TouchManager() = default;
    ~TouchManager();

    bool begin(const JsonObject& config);
    void end();

    ITouchDriver* getActiveDriver() { return active_driver_; }
    const ITouchDriver* getActiveDriver() const { return active_driver_; }
    String getActiveType() const;

    TouchPoint read();
    void setSensitivity(const String& level);
    void setCalibration(const Calibration& calibration);
    TouchPoint applyCalibration(const TouchPoint& point) const;

private:
    static constexpr uint32_t kPressDebounceMs = 50;
    static constexpr uint32_t kReleaseDebounceMs = 100;
    static constexpr int32_t kDragThresholdSquared = 8 * 8;

    ITouchDriver* active_driver_ = nullptr;
    Calibration calibration_;
    TouchPoint filtered_point_;
    TouchPoint candidate_point_;
    uint32_t candidate_since_ms_ = 0;
    bool candidate_valid_ = false;

    bool tryDriver(const String& type, const JsonObject& config);
    void loadCalibration(const JsonObject& config);
};

} // namespace filament_dryer
