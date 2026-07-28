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
 * PWMFeedforwardControl - Implementation
 */
#include "PWMFeedforwardControl.hpp"
#include <Arduino.h>

namespace filament_dryer {

PWMFeedforwardControl::PWMFeedforwardControl() {}

PWMFeedforwardControl::PWMFeedforwardControl(const JsonObject& config) {
    base_pwm_ = config["base_pwm"] | 50.0f;
    temp_coeff_ = config["temp_coefficient"] | 2.5f;
    max_pwm_ = config["max_pwm"] | 100.0f;
    min_pwm_ = config["min_pwm"] | 0.0f;
}

PWMFeedforwardControl::~PWMFeedforwardControl() {}

bool PWMFeedforwardControl::begin(const JsonObject& config) {
    base_pwm_ = config["base_pwm"] | 50.0f;
    temp_coeff_ = config["temp_coefficient"] | 2.5f;
    max_pwm_ = config["max_pwm"] | 100.0f;
    min_pwm_ = config["min_pwm"] | 0.0f;
    
    initialized_ = true;
    
    return true;
}

float PWMFeedforwardControl::compute(float target_temp, float current_temp, float dt) {
    if (!initialized_) return 0.0f;
    
    // Calculate feedforward term based on target temperature
    // PWM = base_pwm + temp_coeff * (target - reference_temp)
    // reference_temp is typically 50°C
    float reference_temp = 50.0f;
    float pwm = base_pwm_ + temp_coeff_ * (target_temp - reference_temp);
    
    // Clamp to limits
    return constrain(pwm, 0.0f, 100.0f);
}

void PWMFeedforwardControl::reset() {
    // Nothing to reset for feedforward
}

JsonObject PWMFeedforwardControl::getParameters() {
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    obj["base_pwm"] = base_pwm_;
    obj["temp_coefficient"] = temp_coeff_;
    obj["max_pwm"] = max_pwm_;
    obj["min_pwm"] = min_pwm_;
    return obj;
}

void PWMFeedforwardControl::setParameters(const JsonObject& params) {
    if (params.containsKey("base_pwm")) base_pwm_ = params["base_pwm"];
    if (params.containsKey("base_pwm_pct")) base_pwm_ = params["base_pwm_pct"];
    if (params.containsKey("temp_coefficient")) temp_coeff_ = params["temp_coefficient"];
    if (params.containsKey("max_pwm")) max_pwm_ = params["max_pwm"];
    if (params.containsKey("min_pwm")) min_pwm_ = params["min_pwm"];
}

} // namespace filament_dryer