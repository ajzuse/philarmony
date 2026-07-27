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
    
    state_ = {};
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = false;
    
    initialized_ = true;
    logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR, 
                     "GPIO Actuator on GPIO %d (%s)", gpio_pin_, active_high_ ? "active high" : "active low");
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
    
    logMgr.logSystem(LogLevel::WARNING, LogModule::ACTUATOR, "GPIO Actuator emergency stop");
}

ActuatorState GPIOActuator::getState() const {
    return state_;
}

void GPIOActuator::applyState(bool on) {
    digitalWrite(gpio_pin_, active_high_ ? (on ? HIGH : LOW) : (on ? LOW : HIGH));
}

} // namespace filament_dryer