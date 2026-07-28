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
    
    gpio_pin_ = config["gpio_pin"] | 25;
    pwm_freq_ = config["pwm_freq"] | 1000;
    max_power_pct_ = config["max_power_pct"] | 100;
    pwm_channel_ = config["pwm_channel"] | 0;
    pwm_resolution_ = config["pwm_resolution"] | 10;
    
    ledcSetup(pwm_channel_, pwm_freq_, pwm_resolution_);
    ledcAttachPin(gpio_pin_, pwm_channel_);
    ledcWrite(pwm_channel_, 0);
    
    state_ = ActuatorState{};
    initialized_ = true;
    
    return true;
}

bool SSRActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    if (state_.fault) return false;
    
    power_pct = constrain(power_pct, 0.0f, (float)max_power_pct_);
    
    uint32_t max_duty = (1 << 10) - 1;
    uint32_t duty = (uint32_t)(power_pct / 100.0f * max_duty);
    ledcWrite(pwm_channel_, duty);
    
    state_.enabled = (power_pct > 0);
    state_.power_pct = power_pct;
    state_.fault = false;
    state_.fault_message = "";
    return true;
}

void SSRActuator::emergencyStop() {
    if (!initialized_) return;
    ledcWrite(pwm_channel_, 0);
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = true;
    state_.fault_message = "Emergency stop";
}

ActuatorState SSRActuator::getState() const {
    return state_;
}

} // namespace filament_dryer