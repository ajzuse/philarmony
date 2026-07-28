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
 * StepperActuator - Implementation
 * Stepper motor driver (A4988, DRV8825, TMC2209, etc.)
 */
#include "StepperActuator.hpp"

namespace filament_dryer {

StepperActuator::StepperActuator() {}

StepperActuator::~StepperActuator() {}

bool StepperActuator::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    step_pin_ = config["step_pin"] | -1;
    dir_pin_ = config["dir_pin"] | -1;
    enable_pin_ = config["enable_pin"] | -1;
    ms1_pin_ = config["ms1_pin"] | -1;
    ms2_pin_ = config["ms2_pin"] | -1;
    ms3_pin_ = config["ms3_pin"] | -1;
    max_speed_ = config["max_speed"] | 1000;
    microsteps_ = config["microsteps"] | 16;
    
    if (step_pin_ < 0 || dir_pin_ < 0) {
        return false;
    }
    
    pinMode(step_pin_, OUTPUT);
    pinMode(dir_pin_, OUTPUT);
    digitalWrite(step_pin_, LOW);
    digitalWrite(dir_pin_, LOW);
    
    if (enable_pin_ >= 0) {
        pinMode(enable_pin_, OUTPUT);
        digitalWrite(enable_pin_, LOW);  // Active low
    }
    
    if (ms1_pin_ >= 0) pinMode(ms1_pin_, OUTPUT);
    if (ms2_pin_ >= 0) pinMode(ms2_pin_, OUTPUT);
    if (ms3_pin_ >= 0) pinMode(ms3_pin_, OUTPUT);
    
    setMicrostepping(microsteps_);
    
    state_ = ActuatorState{};
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = false;
    
    initialized_ = true;
    
    return true;
}

bool StepperActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    if (state_.fault) return false;
    
    power_pct = constrain(power_pct, 0.0f, 100.0f);
    
    if (power_pct > 0) {
        state_.enabled = true;
        state_.power_pct = power_pct;
        
        // Convert power percentage to speed
        float target_speed = (power_pct / 100.0f) * max_speed_;
        setSpeed(target_speed);
    } else {
        state_.enabled = false;
        state_.power_pct = 0.0f;
    }
    
    return true;
}

void StepperActuator::emergencyStop() {
    if (!initialized_) return;
    
    current_speed_ = 0;
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = true;
    state_.fault_message = "Emergency stop";
    
}

ActuatorState StepperActuator::getState() const {
    return state_;
}

bool StepperActuator::moveSteps(int32_t steps) {
    if (!initialized_ || !state_.enabled) return false;
    
    target_position_ = current_position_ + steps;
    setDirection(steps > 0);
    
    // Simple blocking move (in production, use timer interrupt)
    for (int32_t i = 0; i < abs(steps); i++) {
        if (state_.fault) break;
        stepPulse();
        current_position_ += (steps > 0) ? 1 : -1;
        delayMicroseconds(1000000 / max(1.0f, current_speed_));
    }
    
    current_position_ = target_position_;
    return true;
}

bool StepperActuator::setSpeed(float steps_per_sec) {
    if (!initialized_) return false;
    
    current_speed_ = constrain(steps_per_sec, 0.0f, (float)max_speed_);
    return true;
}

bool StepperActuator::home() {
    if (!initialized_) return false;
    
    // Simple homing: move negative until limit switch (not implemented)
    // In practice, would use limit switch on a dedicated pin
    return true;
}

void StepperActuator::setMicrostepping(uint8_t ms) {
    microsteps_ = constrain((uint8_t)ms, 1, 256);
    
    // Set microstepping pins (MS1, MS2, MS3)
    // 1: LOW, LOW, LOW
    // 2: HIGH, LOW, LOW
    // 4: LOW, HIGH, LOW
    // 8: HIGH, HIGH, LOW
    // 16: HIGH, HIGH, HIGH
    
    if (ms1_pin_ >= 0) digitalWrite(ms1_pin_, (microsteps_ >= 2) ? HIGH : LOW);
    if (ms2_pin_ >= 0) digitalWrite(ms2_pin_, (microsteps_ >= 4) ? HIGH : LOW);
    if (ms3_pin_ >= 0) digitalWrite(ms3_pin_, (microsteps_ >= 16) ? HIGH : LOW);
}

void StepperActuator::setDirection(bool cw) {
    digitalWrite(dir_pin_, cw ? HIGH : LOW);
}

void StepperActuator::stepPulse() {
    digitalWrite(step_pin_, HIGH);
    delayMicroseconds(1);
    digitalWrite(step_pin_, LOW);
    delayMicroseconds(1);
}

} // namespace filament_dryer