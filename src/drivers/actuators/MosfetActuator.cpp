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
 * MosfetActuator - Implementation
 * AOD4184 N-channel MOSFET PWM heater control with PID support
 */
#include "MosfetActuator.hpp"

namespace filament_dryer {

MosfetActuator::MosfetActuator() {}

MosfetActuator::~MosfetActuator() {
    if (initialized_) {
        emergencyStop();
    }
}

bool MosfetActuator::begin(const JsonObject& config) {
    gpio_pin_ = config["heater_pin"] | config["pwm"] | config["gpio_pin"] | 25;
    pwm_freq_ = config["heater_pwm_freq"] | 1000;
    max_power_pct_ = config["heater_max_power_pct"] | 100;
    pwm_channel_ = config["pwm_channel"] | 0;
    pwm_resolution_ = config["pwm_resolution"] | 10;
    current_sense_pin_ = config["current_sense_pin"] | config["sense_pin"] | -1;
    overcurrent_adc_threshold_ = config["overcurrent_adc_threshold"] | 3000;
    open_loop_detect_ = config["open_loop_detect"] | false;
    overcurrent_latched_ = false;
    emergency_stopped_ = false;
    measured_power_pct_ = 0.0f;
    
    // Configure LEDC PWM
    ledcSetup(pwm_channel_, pwm_freq_, pwm_resolution_);
    ledcAttachPin(gpio_pin_, pwm_channel_);
    
    // Ensure off at start
    ledcWrite(pwm_channel_, 0);

    if (current_sense_pin_ >= 0) {
        pinMode(current_sense_pin_, INPUT);
    }
    
    state_ = ActuatorState{};
    initialized_ = true;
    return true;
}

bool MosfetActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    
    // Clamp to safety limits
    power_pct = constrain(power_pct, 0.0f, (float)max_power_pct_);
    
    if (state_.fault && !emergency_stopped_) {
        return false;
    }
    if (overcurrent_latched_) {
        return false;
    }
    // Allow recovery from emergency stop when commanding again
    if (emergency_stopped_ && power_pct >= 0.0f) {
        emergency_stopped_ = false;
        state_.fault = false;
        state_.fault_message = "";
    }
    
    state_.power_pct = power_pct;
    state_.enabled = (power_pct > 0.0f);
    
    applyDuty(power_to_duty(power_pct));

    // Open-loop heuristic: when enabled without sense pin, treat commanded echo as measured
    // only if open_loop_detect_ — otherwise measured stays unset (no false PWM mismatch).
    if (hasCurrentSense()) {
        checkOvercurrent();
    } else if (open_loop_detect_) {
        // Stub open-loop: without ADC, cannot trip on current; leave measured unset.
        measured_power_pct_ = state_.power_pct;
    }
    
    return true;
}

bool MosfetActuator::checkOvercurrent() {
    if (!initialized_ || current_sense_pin_ < 0) {
        return false;
    }
    const int adc = analogRead(current_sense_pin_);
    // Map ADC roughly to "measured" power for feedback comparison when sensing shunt
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

void MosfetActuator::emergencyStop() {
    if (!initialized_) return;
    
    // Immediate hardware cutoff
    ledcWrite(pwm_channel_, 0);
    
    state_.enabled = false;
    state_.power_pct = 0.0f;
    emergency_stopped_ = true;
    // Do NOT set state_.fault for e-stop — that must not count as overcurrent (T118)
    state_.fault_message = "Emergency stop";
    
}

ActuatorState MosfetActuator::getState() const {
    return state_;
}

void MosfetActuator::setPidConfig(float kp, float ki, float kd) {
    pid_kp_ = kp;
    pid_ki_ = ki;
    pid_kd_ = kd;
    pid_enabled_ = (kp > 0.0f || ki > 0.0f || kd > 0.0f);
    resetPid();
    
}

float MosfetActuator::computePid(float target_temp, float current_temp, float dt) {
    if (!pid_enabled_ || dt <= 0.0f) return 0.0f;
    
    float error = target_temp - current_temp;
    
    // Proportional term
    float p_term = pid_kp_ * error;
    
    // Integral term with anti-windup
    pid_integral_ += error * dt;
    // Anti-windup: clamp integral to reasonable range
    pid_integral_ = constrain(pid_integral_, -1000.0f, 1000.0f);
    float i_term = pid_ki_ * pid_integral_;
    
    // Derivative term
    float d_term = pid_kd_ * (error - pid_last_error_) / dt;
    
    float output = p_term + i_term + d_term;
    
    // Clamp output to valid power range
    output = constrain(output, 0.0f, (float)max_power_pct_);
    
    pid_last_error_ = error;
    
    return output;
}

void MosfetActuator::resetPid() {
    pid_integral_ = 0.0f;
    pid_last_error_ = 0.0f;
    pid_last_time_ = millis();
}

float MosfetActuator::power_to_duty(float power_pct) const {
    // 10-bit resolution = 1024 steps
    return (power_pct / 100.0f) * 1023.0f;
}

void MosfetActuator::applyDuty(uint32_t duty) {
    ledcWrite(pwm_channel_, duty);
}

} // namespace filament_dryer