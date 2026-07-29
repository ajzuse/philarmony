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
 * SSRActuator - Implementation
 */
#include "SSRActuator.hpp"

namespace filament_dryer {

SSRActuator::SSRActuator() {}

SSRActuator::~SSRActuator() {
    if (initialized_) emergencyStop();
}

bool SSRActuator::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    gpio_pin_ = config["gpio_pin"] | config["heater_pin"] | config["pwm"] | 25;
    pwm_freq_ = config["pwm_freq"] | config["heater_pwm_freq"] | 1000;
    max_power_pct_ = config["max_power_pct"] | config["heater_max_power_pct"] | 100;
    pwm_channel_ = config["pwm_channel"] | 0;
    pwm_resolution_ = config["pwm_resolution"] | 10;
    current_sense_pin_ = config["current_sense_pin"] | config["sense_pin"] | -1;
    overcurrent_adc_threshold_ = config["overcurrent_adc_threshold"] | 3000;
    open_loop_detect_ = config["open_loop_detect"] | false;
    open_loop_sense_pin_ = config["open_loop_sense_pin"] | -1;
    inject_measured_power_pct_ = config["inject_measured_power_pct"] | -1.0f;
    overcurrent_latched_ = false;
    emergency_stopped_ = false;
    measured_power_pct_ = 0.0f;
    
    ledcSetup(pwm_channel_, pwm_freq_, pwm_resolution_);
    ledcAttachPin(gpio_pin_, pwm_channel_);
    ledcWrite(pwm_channel_, 0);

    if (current_sense_pin_ >= 0) {
        pinMode(current_sense_pin_, INPUT);
    }
    if (open_loop_sense_pin_ >= 0) {
        pinMode(open_loop_sense_pin_, INPUT);
    }
    
    state_ = ActuatorState{};
    initialized_ = true;
    
    return true;
}

bool SSRActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    if (overcurrent_latched_) return false;
    if (state_.fault && !emergency_stopped_) return false;

    if (emergency_stopped_ && power_pct >= 0.0f) {
        emergency_stopped_ = false;
        state_.fault = false;
        state_.fault_message = "";
    }
    
    power_pct = constrain(power_pct, 0.0f, (float)max_power_pct_);
    
    uint32_t max_duty = (1u << pwm_resolution_) - 1u;
    uint32_t duty = (uint32_t)(power_pct / 100.0f * max_duty);
    applyDuty(duty);
    
    state_.enabled = (power_pct > 0);
    state_.power_pct = power_pct;
    state_.fault = false;
    state_.fault_message = "";

    if (current_sense_pin_ >= 0) {
        checkOvercurrent();
    } else if (open_loop_detect_) {
        refreshOpenLoopMeasurement();
    }
    return true;
}

void SSRActuator::refreshOpenLoopMeasurement() {
    if (inject_measured_power_pct_ >= 0.0f) {
        measured_power_pct_ = inject_measured_power_pct_;
        return;
    }
    if (open_loop_sense_pin_ >= 0) {
        const int adc = analogRead(open_loop_sense_pin_);
        measured_power_pct_ = constrain((adc / 4095.0f) * 100.0f, 0.0f, 100.0f);
        return;
    }
    const uint32_t max_duty = (1u << pwm_resolution_) - 1u;
    const uint32_t duty = ledcRead(pwm_channel_);
    measured_power_pct_ = max_duty > 0
        ? constrain((duty * 100.0f) / static_cast<float>(max_duty), 0.0f, 100.0f)
        : 0.0f;
}

bool SSRActuator::checkOvercurrent() {
    if (!initialized_ || current_sense_pin_ < 0) {
        return false;
    }
    const int adc = analogRead(current_sense_pin_);
    measured_power_pct_ = constrain((adc / 4095.0f) * 100.0f, 0.0f, 100.0f);
    if (adc >= static_cast<int>(overcurrent_adc_threshold_) && state_.power_pct > 5.0f) {
        overcurrent_latched_ = true;
        ledcWrite(pwm_channel_, 0);
        state_.enabled = false;
        state_.power_pct = 0.0f;
        state_.fault = true;
        state_.fault_message = "Overcurrent sense";
        return true;
    }
    return false;
}

void SSRActuator::emergencyStop() {
    if (!initialized_) return;
    ledcWrite(pwm_channel_, 0);
    state_.enabled = false;
    state_.power_pct = 0.0f;
    measured_power_pct_ = 0.0f;
    emergency_stopped_ = true;
    // Do not set fault — e-stop must not count as overcurrent
    state_.fault_message = "Emergency stop";
}

ActuatorState SSRActuator::getState() const {
    return state_;
}

void SSRActuator::applyDuty(uint32_t duty) {
    ledcWrite(pwm_channel_, duty);
}

} // namespace filament_dryer
