/**
 * PWMFeedforwardControl - Implementation
 */
#include "PWMFeedforwardControl.hpp"
#include "../core/LogManager.hpp"

extern filament_dryer::LogManager logMgr;

namespace filament_dryer {

PWMFeedforwardControl::PWMFeedforwardControl() {}

PWMFeedforwardControl::~PWMFeedforwardControl() {}

bool PWMFeedforwardControl::begin(const JsonObject& config) {
    base_pwm_ = config["base_pwm"] | 50.0f;
    temp_coeff_ = config["temp_coefficient"] | 2.5f;
    max_pwm_ = config["max_pwm"] | 100.0f;
    min_pwm_ = config["min_pwm"] | 0.0f;
    
    initialized_ = true;
    
    logMgr.logSystem(LogLevel::INFO, LogModule::CONTROL, 
                     "PWM Feedforward Control initialized: base_pwm=%.1f%%, coeff=%.2f",
                     base_pwm_, temp_coeff_);
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

} // namespace filament_dryer