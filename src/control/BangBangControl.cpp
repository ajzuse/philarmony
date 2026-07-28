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
 * BangBangControl - Implementation
 * Hysteresis-based on/off temperature control
 */
#include <Arduino.h>
#include "BangBangControl.hpp"

namespace filament_dryer {

BangBangControl::BangBangControl() {}

BangBangControl::BangBangControl(const JsonObject& config) {
    config_.hysteresis = config["hysteresis"] | 1.0f;
    config_.min_cycle_time = config["min_cycle_time"] | 10.0f;
    
    if (config_.hysteresis <= 0) config_.hysteresis = 1.0f;
    if (config_.min_cycle_time <= 0) config_.min_cycle_time = 10.0f;
}

BangBangControl::~BangBangControl() {}

bool BangBangControl::begin(const JsonObject& config) {
    config_.hysteresis = config["hysteresis"] | 1.0f;
    config_.min_cycle_time = config["min_cycle_time"] | 10.0f;
    
    if (config_.hysteresis <= 0) config_.hysteresis = 1.0f;
    if (config_.min_cycle_time <= 0) config_.min_cycle_time = 10.0f;
    
    reset();
    initialized_ = true;
    
    return true;
}

float BangBangControl::compute(float target, float current, float dt) {
    if (!initialized_) return 0.0f;
    
    uint32_t now = millis();
    float time_since_switch = (now - last_switch_time_) / 1000.0f;
    
    // Check if we should switch
    if (shouldSwitch(target, current)) {
        // Enforce minimum cycle time
        if (time_since_switch >= config_.min_cycle_time) {
            last_output_ = (last_output_ > 0) ? 0.0f : 100.0f;
            last_switch_time_ = millis();
        }
    }
    
    return last_output_;
}

bool BangBangControl::shouldSwitch(float target, float current) {
    if (last_output_ > 0) {
        // Heater is ON, switch OFF if temp >= target + hysteresis/2
        return current >= (target + config_.hysteresis / 2.0f);
    } else {
        // Heater is OFF, switch ON if temp <= target - hysteresis/2
        return current <= (target - config_.hysteresis / 2.0f);
    }
}

void BangBangControl::reset() {
    last_output_ = 0.0f;
    last_switch_time_ = millis();
}

JsonObject BangBangControl::getParameters() {
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    obj["hysteresis"] = config_.hysteresis;
    obj["min_cycle_time"] = config_.min_cycle_time;
    return obj;
}

void BangBangControl::setParameters(const JsonObject& params) {
    if (params.containsKey("hysteresis")) config_.hysteresis = params["hysteresis"];
    if (params.containsKey("min_cycle_time")) config_.min_cycle_time = params["min_cycle_time"];
    if (config_.hysteresis <= 0) config_.hysteresis = 1.0f;
    if (config_.min_cycle_time <= 0) config_.min_cycle_time = 10.0f;
    reset();
}

} // namespace filament_dryer
