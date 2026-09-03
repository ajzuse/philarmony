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
 * TouchManager - Implementation
 */
#include "TouchManager.hpp"

#include "Cst816sTouch.hpp"
#include "Ft6236Touch.hpp"
#include "Gt911Touch.hpp"
#include "Stmpe610Touch.hpp"
#include "Xpt2046Touch.hpp"

namespace filament_dryer {

namespace {
constexpr const char* kI2cProbeOrder[] = {"ft6236", "gt911", "cst816s",
                                          "stmpe610"};
constexpr size_t kI2cProbeCount = sizeof(kI2cProbeOrder) / sizeof(kI2cProbeOrder[0]);
}

TouchManager::~TouchManager() {
    end();
}

bool TouchManager::begin(const JsonObject& config) {
    end();
    last_config_json_.clear();
    serializeJson(config, last_config_json_);
    last_watchdog_ms_ = millis();
    loadCalibration(config);

    String type = config["controller_type"] | "auto";
    if (!config.containsKey("controller_type") && config.containsKey("driver")) {
        type = config["driver"].as<String>();
    }

    if (type == "none") {
        return false;
    }

    if (type == "auto") {
        for (size_t i = 0; i < kI2cProbeCount; ++i) {
            if (tryDriver(kI2cProbeOrder[i], config)) {
                break;
            }
        }

        const int8_t spi_cs = config["spi_cs"] | -1;
        if (!active_driver_ && spi_cs >= 0) {
            tryDriver("xpt2046", config);
        }
    } else {
        tryDriver(type, config);
    }

    if (active_driver_) {
        setSensitivity(config["sensitivity"] | "medium");
        last_watchdog_ms_ = millis();
    }
    return active_driver_ != nullptr;
}

void TouchManager::serviceWatchdog() {
    if (!active_driver_ || last_config_json_.isEmpty()) {
        return;
    }
    if (millis() - last_watchdog_ms_ < kWatchdogMs) {
        return;
    }
    last_watchdog_ms_ = millis();
    if (active_driver_->isConnected()) {
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, last_config_json_) != DeserializationError::Ok) {
        return;
    }
    begin(doc.as<JsonObject>());
}

void TouchManager::end() {
    delete active_driver_;
    active_driver_ = nullptr;
    filtered_point_ = TouchPoint{};
    candidate_point_ = TouchPoint{};
    candidate_since_ms_ = 0;
    candidate_valid_ = false;
}

String TouchManager::getActiveType() const {
    return active_driver_ ? active_driver_->getType() : String("none");
}

TouchPoint TouchManager::read() {
    if (!active_driver_) {
        return TouchPoint{};
    }

    TouchPoint raw = applyCalibration(active_driver_->read());
    if (raw.timestamp_ms == 0) {
        raw.timestamp_ms = millis();
    }

    if (raw.pressed != filtered_point_.pressed) {
        if (!candidate_valid_ || candidate_point_.pressed != raw.pressed) {
            candidate_point_ = raw;
            candidate_since_ms_ = raw.timestamp_ms;
            candidate_valid_ = true;
            return filtered_point_;
        }

        candidate_point_ = raw;
        const uint32_t debounce_ms =
            raw.pressed ? kPressDebounceMs : kReleaseDebounceMs;
        if (raw.timestamp_ms - candidate_since_ms_ < debounce_ms) {
            return filtered_point_;
        }

        if (raw.pressed) {
            filtered_point_ = raw;
        } else {
            filtered_point_.pressed = false;
            filtered_point_.timestamp_ms = raw.timestamp_ms;
        }
        candidate_valid_ = false;
        return filtered_point_;
    }

    candidate_valid_ = false;
    if (raw.pressed) {
        const int32_t dx = static_cast<int32_t>(raw.x) - filtered_point_.x;
        const int32_t dy = static_cast<int32_t>(raw.y) - filtered_point_.y;
        if ((dx * dx) + (dy * dy) >= kDragThresholdSquared) {
            filtered_point_.x = raw.x;
            filtered_point_.y = raw.y;
            filtered_point_.timestamp_ms = raw.timestamp_ms;
        }
    }
    return filtered_point_;
}

void TouchManager::setSensitivity(const String& level) {
    if (!active_driver_) {
        return;
    }

    if (level == "low" || level == "high") {
        active_driver_->setSensitivity(level);
    } else {
        active_driver_->setSensitivity("medium");
    }
}

void TouchManager::setCalibration(const Calibration& calibration) {
    calibration_ = calibration;
}

TouchPoint TouchManager::applyCalibration(const TouchPoint& point) const {
    if (!point.pressed || !calibration_.isValid()) {
        return point;
    }

    TouchPoint calibrated = point;
    const int32_t max_x = calibration_.width - 1;
    const int32_t max_y = calibration_.height - 1;
    int32_t x = (static_cast<int32_t>(point.x) - calibration_.x_min) * max_x /
                (calibration_.x_max - calibration_.x_min);
    int32_t y = (static_cast<int32_t>(point.y) - calibration_.y_min) * max_y /
                (calibration_.y_max - calibration_.y_min);

    x = constrain(x, static_cast<int32_t>(0), max_x);
    y = constrain(y, static_cast<int32_t>(0), max_y);
    if (calibration_.invert_x) {
        x = max_x - x;
    }
    if (calibration_.invert_y) {
        y = max_y - y;
    }
    if (calibration_.swap_xy) {
        const int32_t old_x = x;
        x = constrain(y, static_cast<int32_t>(0), max_x);
        y = constrain(old_x, static_cast<int32_t>(0), max_y);
    }

    calibrated.x = static_cast<int16_t>(x);
    calibrated.y = static_cast<int16_t>(y);
    return calibrated;
}

bool TouchManager::tryDriver(const String& type, const JsonObject& config) {
    ITouchDriver* driver = nullptr;
    if (type == "ft6236") {
        driver = new Ft6236Touch();
    } else if (type == "gt911") {
        driver = new Gt911Touch();
    } else if (type == "cst816s") {
        driver = new Cst816sTouch();
    } else if (type == "xpt2046") {
        driver = new Xpt2046Touch();
    } else if (type == "stmpe610") {
        driver = new Stmpe610Touch();
    }

    if (!driver) {
        return false;
    }
    JsonDocument patched;
    for (JsonPair p : config) {
        patched[p.key()] = p.value();
    }
    patched["controller_type"] = type;
    if (type == "stmpe610" &&
        (config["controller_type"] | String("auto")) == "auto") {
        patched["bus"] = "i2c";
    }
    if (!driver->begin(patched.as<JsonObject>()) || !driver->isConnected()) {
        delete driver;
        return false;
    }

    delete active_driver_;
    active_driver_ = driver;
    return true;
}

void TouchManager::loadCalibration(const JsonObject& config) {
    calibration_ = Calibration{};
    calibration_.width = config["display_width"] | (config["width"] | 0);
    calibration_.height = config["display_height"] | (config["height"] | 0);
    calibration_.swap_xy = config["swap_xy"] | false;
    calibration_.invert_x = config["invert_x"] | false;
    calibration_.invert_y = config["invert_y"] | false;

    if (!config.containsKey("calibration")) {
        return;
    }

    JsonObject values = config["calibration"].as<JsonObject>();
    calibration_.x_min = values["x_min"] | 0;
    calibration_.x_max = values["x_max"] | 0;
    calibration_.y_min = values["y_min"] | 0;
    calibration_.y_max = values["y_max"] | 0;
    calibration_.swap_xy = values["swapped_xy"] | calibration_.swap_xy;
}

} // namespace filament_dryer
