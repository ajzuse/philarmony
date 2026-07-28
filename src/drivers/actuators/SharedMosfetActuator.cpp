/**
 * SharedMosfetActuator - Implementation
 * Heater and exhaust fan sharing single MOSFET output
 */
#include "SharedMosfetActuator.hpp"

namespace filament_dryer {

SharedMosfetActuator::SharedMosfetActuator() {}

SharedMosfetActuator::~SharedMosfetActuator() {}

bool SharedMosfetActuator::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    pwm_pin_ = config["heater_pin"] | 25;
    pwm_freq_ = config["pwm_freq"] | 1000;
    pwm_channel_ = config["pwm_channel"] | 0;
    pwm_resolution_ = config["pwm_resolution"] | 10;
    max_power_pct_ = config["max_power_pct"] | 100;
    
    ledcSetup(pwm_channel_, pwm_freq_, pwm_resolution_);
    ledcAttachPin(pwm_pin_, pwm_channel_);
    ledcWrite(pwm_channel_, 0);
    
    state_ = ActuatorState{};
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = false;
    heater_power_ = 0;
    fan_power_ = 0;
    
    initialized_ = true;
    
    return true;
}

bool SharedMosfetActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    if (state_.fault) return false;
    
    // In shared mode, power_pct applies to both heater and fan proportionally
    // For simplicity, we use a fixed ratio: 80% heater, 20% fan
    float heater_ratio = 0.8f;
    float fan_ratio = 0.2f;
    
    return setHeaterPower(power_pct * heater_ratio) && 
           setFanPower(power_pct * fan_ratio);
}

void SharedMosfetActuator::emergencyStop() {
    if (!initialized_) return;
    
    ledcWrite(pwm_channel_, 0);
    
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = true;
    state_.fault_message = "Emergency stop";
    heater_power_ = 0;
    fan_power_ = 0;
    
}

ActuatorState SharedMosfetActuator::getState() const {
    return state_;
}

bool SharedMosfetActuator::setHeaterPower(float power_pct) {
    if (!initialized_) return false;
    if (state_.fault) return false;
    
    power_pct = constrain(power_pct, 0.0f, (float)max_power_pct_);
    
    // Apply combined power (heater takes priority in shared mode)
    float combined_power = power_pct + fan_power_;
    if (combined_power > 100.0f) {
        // Scale down proportionally
        float scale = 100.0f / combined_power;
        heater_power_ = power_pct * scale;
        fan_power_ = fan_power_ * scale;
    } else {
        heater_power_ = power_pct;
    }
    
    applyPWM(heater_power_ + fan_power_);
    
    state_.enabled = (heater_power_ + fan_power_) > 0;
    state_.power_pct = heater_power_ + fan_power_;
    
    return true;
}

bool SharedMosfetActuator::setFanPower(float power_pct) {
    if (!initialized_) return false;
    if (state_.fault) return false;
    
    power_pct = constrain(power_pct, 0.0f, 100.0f);
    
    float combined_power = heater_power_ + power_pct;
    if (combined_power > 100.0f) {
        float scale = 100.0f / combined_power;
        fan_power_ = power_pct * scale;
        heater_power_ = heater_power_ * scale;
    } else {
        fan_power_ = power_pct;
    }
    
    applyPWM(heater_power_ + fan_power_);
    
    state_.enabled = (heater_power_ + fan_power_) > 0;
    state_.power_pct = heater_power_ + fan_power_;
    
    return true;
}

void SharedMosfetActuator::applyPWM(float power_pct) {
    power_pct = constrain(power_pct, 0.0f, 100.0f);
    uint32_t duty = (uint32_t)((power_pct / 100.0f) * ((1 << pwm_resolution_) - 1));
    ledcWrite(pwm_channel_, duty);
}

} // namespace filament_dryer