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
 * PIDControl - Implementation
 * PID temperature control with anti-windup and output clamping
 */
#include "PIDControl.hpp"
#include <Arduino.h>

namespace filament_dryer {

PIDControl::PIDControl() {}

PIDControl::PIDControl(const JsonObject& config) {
    config_.kp = config["kp"] | 0.0f;
    config_.ki = config["ki"] | 0.0f;
    config_.kd = config["kd"] | 0.0f;
    config_.max_integral = config["max_integral"] | 1000.0f;
    config_.min_output = config["min_output"] | 0.0f;
    config_.max_output = config["max_output"] | 100.0f;
}

PIDControl::~PIDControl() {}

bool PIDControl::begin(const JsonObject& config) {
    config_.kp = config["kp"] | 0.0f;
    config_.ki = config["ki"] | 0.0f;
    config_.kd = config["kd"] | 0.0f;
    config_.max_integral = config["max_integral"] | 1000.0f;
    config_.min_output = config["min_output"] | 0.0f;
    config_.max_output = config["max_output"] | 100.0f;
    
    reset();
    initialized_ = true;
    
    return true;
}

float PIDControl::compute(float target, float current, float dt) {
    if (!initialized_ || dt <= 0) return 0.0f;
    
    float error = target - current;
    
    // Proportional term
    float p_term = config_.kp * error;
    
    // Integral term with anti-windup
    integral_ += error * dt;
    integral_ = constrain(integral_, -config_.max_integral, config_.max_integral);
    float i_term = config_.ki * integral_;
    
    // Derivative term
    float d_term = config_.kd * (error - last_error_) / dt;
    last_error_ = error;
    
    // Combine terms
    float output = p_term + i_term + d_term;
    
    // Clamp output
    output = clampOutput(output);
    
    // Anti-windup: if output is saturated, limit integral
    if (output >= config_.max_output && i_term > 0) {
        integral_ -= error * dt;
    } else if (output <= config_.min_output && i_term < 0) {
        integral_ -= error * dt;
    }
    
    last_output_ = output;
    return output;
}

void PIDControl::reset() {
    integral_ = 0.0f;
    last_error_ = 0.0f;
    last_output_ = 0.0f;
}

JsonObject PIDControl::getParameters() {
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    obj["kp"] = config_.kp;
    obj["ki"] = config_.ki;
    obj["kd"] = config_.kd;
    obj["max_integral"] = config_.max_integral;
    obj["min_output"] = config_.min_output;
    obj["max_output"] = config_.max_output;
    return obj;
}

void PIDControl::setParameters(const JsonObject& params) {
    if (params.containsKey("kp")) config_.kp = params["kp"];
    if (params.containsKey("ki")) config_.ki = params["ki"];
    if (params.containsKey("kd")) config_.kd = params["kd"];
    if (params.containsKey("max_integral")) config_.max_integral = params["max_integral"];
    if (params.containsKey("min_output")) config_.min_output = params["min_output"];
    if (params.containsKey("max_output")) config_.max_output = params["max_output"];
    reset();
}

float PIDControl::clampOutput(float output) {
    return constrain(output, config_.min_output, config_.max_output);
}

} // namespace filament_dryer