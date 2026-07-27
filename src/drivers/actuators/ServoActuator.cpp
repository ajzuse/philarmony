/**
 * ServoActuator - Implementation
 * Servo motor driver using LEDC PWM
 */
#include "ServoActuator.hpp"

namespace filament_dryer {

ServoActuator::ServoActuator() {}

ServoActuator::~ServoActuator() {}

bool ServoActuator::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    pwm_pin_ = config["pwm_pin"] | 25;
    enable_pin_ = config["enable_pin"] | -1;
    pwm_freq_ = config["pwm_freq"] | 50;
    pwm_channel_ = config["pwm_channel"] | 0;
    pwm_resolution_ = config["pwm_resolution"] | 12;
    min_pulse_ = config["min_pulse"] | 500;
    max_pulse_ = config["max_pulse"] | 2500;
    min_angle_ = config["min_angle"] | 0.0f;
    max_angle_ = config["max_angle"] | 180.0f;

    // Configure LEDC PWM
    ledcSetup(pwm_channel_, 50, 12);
    ledcAttachPin(pwm_pin_, pwm_channel_);

    if (enable_pin_ >= 0) {
        pinMode(enable_pin_, OUTPUT);
        digitalWrite(enable_pin_, HIGH);
    }

    state_ = {};
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = false;

    initialized_ = true;
    logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR, 
                     "Servo initialized on GPIO %d (channel %d, %dHz)", 
                     pwm_pin_, pwm_channel_, pwm_freq_);
    return true;
}

bool ServoActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    if (state_.fault) return false;

    power_pct = constrain(power_pct, 0.0f, 100.0f);
    float angle = map(power_pct, 0.0f, 100.0f, min_angle_, max_angle_);
    
    uint16_t pulse = angleToPulse(angle);
    ledcWrite(pwm_channel_, pulse);
    
    state_.enabled = (power_pct > 0);
    state_.power_pct = power_pct;
    
    return true;
}

void ServoActuator::emergencyStop() {
    if (!initialized_) return;
    
    // Move to safe position (0° or min_angle)
    ledcWrite(pwm_channel_, angleToPulse(min_angle_));
    
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = true;
    state_.fault_message = "Emergency stop";
    
    logMgr.logSystem(LogLevel::WARNING, LogModule::ACTUATOR, "Servo emergency stop");
}

ActuatorState ServoActuator::getState() const {
    return state_;
}

uint16_t ServoActuator::angleToPulse(float angle) const {
    // Map angle to pulse width in microseconds
    float ratio = (angle - min_angle_) / (max_angle_ - min_angle_);
    ratio = constrain(ratio, 0.0f, 1.0f);
    return (uint16_t)(min_pulse_ + ratio * (max_pulse_ - min_pulse_));
}

} // namespace filament_dryer