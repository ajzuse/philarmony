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
 * Fan Actuator - Implementation
 */
#include "FanActuator.hpp"

namespace filament_dryer {

FanActuator::FanActuator() {}

FanActuator::~FanActuator() {}

bool FanActuator::begin(const JsonObject& config) {
    gpio_pin_ = config["fan_pin"] | config["gpio_pin"] | 26;
    pwm_freq_ = config["fan_pwm_freq"] | 5000;
    type_ = config["type"] | "fan_pwm";
    String mode_str = config["fan_mode"] | "independent_pwm";
    if (type_ == "fan_digital") {
        mode_str = "independent_digital";
    }
    mode_ = parseFanMode(mode_str);
    if (mode_ == FanMode::INDEPENDENT_DIGITAL) {
        type_ = "fan_digital";
    }
    shared_heater_pin_ = config["heater_pin"] | -1;
    
    // Configure PWM channel for independent modes
    if (mode_ == FanMode::INDEPENDENT_PWM) {
        pwm_channel_ = 1; // Channel 1 (Channel 0 for heater)
        pwm_resolution_ = 10; // 10-bit = 0-1023
        
        ledcSetup(pwm_channel_, pwm_freq_, pwm_resolution_);
        ledcAttachPin(gpio_pin_, pwm_channel_);
        ledcWrite(pwm_channel_, 0);
        
    } else if (mode_ == FanMode::INDEPENDENT_DIGITAL) {
        pinMode(gpio_pin_, OUTPUT);
        digitalWrite(gpio_pin_, LOW);
    }
    
    state_ = ActuatorState{};
    initialized_ = true;
    return true;
}

FanActuator::FanMode FanActuator::parseFanMode(const String& mode_str) {
    if (mode_str == "shared_mosfet") return FanMode::SHARED_MOSFET;
    if (mode_str == "independent_digital") return FanMode::INDEPENDENT_DIGITAL;
    return FanMode::INDEPENDENT_PWM;
}

float FanActuator::power_to_duty(float power_pct) const {
    return constrain(power_pct, 0.0f, 100.0f) / 100.0f;
}

void FanActuator::applyDuty(uint32_t duty) {
    if (mode_ == FanMode::INDEPENDENT_PWM) {
        ledcWrite(pwm_channel_, duty);
    } else if (mode_ == FanMode::INDEPENDENT_DIGITAL) {
        digitalWrite(gpio_pin_, duty > 0 ? HIGH : LOW);
    }
    // Shared MOSFET mode: fan follows heater, no direct control
}

bool FanActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    
    // In shared MOSFET mode, fan follows heater power
    if (mode_ == FanMode::SHARED_MOSFET) {
        state_.enabled = (power_pct > 0);
        state_.power_pct = power_pct;
        return true;
    }
    
    power_pct = constrain(power_pct, 0.0f, 100.0f);
    float duty = power_to_duty(power_pct);
    uint32_t duty_raw = (uint32_t)(duty * ((1 << pwm_resolution_) - 1));
    
    applyDuty(duty_raw);
    
    state_.enabled = (power_pct > 0);
    state_.power_pct = power_pct;
    state_.fault = false;
    state_.fault_message = "";
    
    return true;
}

void FanActuator::emergencyStop() {
    if (!initialized_) return;
    
    if (mode_ == FanMode::INDEPENDENT_PWM) {
        ledcWrite(pwm_channel_, 0);
    } else if (mode_ == FanMode::INDEPENDENT_DIGITAL) {
        digitalWrite(gpio_pin_, LOW);
    }
    
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = false;
    state_.fault_message = "Emergency stop";
}

ActuatorState FanActuator::getState() const {
    return state_;
}

} // namespace filament_dryer