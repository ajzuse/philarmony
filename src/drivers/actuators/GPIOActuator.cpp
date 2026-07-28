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
 * GPIOActuator - Implementation
 */
#include "GPIOActuator.hpp"

namespace filament_dryer {

GPIOActuator::GPIOActuator() {}

GPIOActuator::~GPIOActuator() {}

bool GPIOActuator::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    gpio_pin_ = config["gpio_pin"] | 26;
    active_high_ = config["active_high"] | true;
    
    pinMode(gpio_pin_, OUTPUT);
    applyState(false);
    
    state_ = ActuatorState{};
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = false;
    
    initialized_ = true;
    return true;
}

bool GPIOActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    if (state_.fault) return false;
    
    bool on = power_pct > 0.0f;
    applyState(on);
    
    state_.enabled = on;
    state_.power_pct = on ? 100.0f : 0.0f;
    return true;
}

void GPIOActuator::emergencyStop() {
    if (!initialized_) return;
    
    applyState(false);
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = true;
    state_.fault_message = "Emergency stop";
    
}

ActuatorState GPIOActuator::getState() const {
    return state_;
}

void GPIOActuator::applyState(bool on) {
    digitalWrite(gpio_pin_, active_high_ ? (on ? HIGH : LOW) : (on ? LOW : HIGH));
}

} // namespace filament_dryer