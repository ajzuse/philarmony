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
    gpio_pin_ = config["heater_pin"] | 25;
    pwm_freq_ = config["heater_pwm_freq"] | 1000;
    max_power_pct_ = config["heater_max_power_pct"] | 100;
    pwm_channel_ = config["pwm_channel"] | 0;
    pwm_resolution_ = config["pwm_resolution"] | 10;
    
    // Configure LEDC PWM
    ledcSetup(pwm_channel_, pwm_freq_, pwm_resolution_);
    ledcAttachPin(gpio_pin_, pwm_channel_);
    
    // Ensure off at start
    ledcWrite(pwm_channel_, 0);
    
    state_ = {false, 0.0f, false, ""};
    initialized_ = true;
    
    Serial.printf("[MosfetActuator] AOD4184 on GPIO %d, freq=%luHz, ch=%d, max=%d%%\n",
                  gpio_pin_, pwm_freq_, pwm_channel_, max_power_pct_);
    
    return true;
}

bool MosfetActuator::setPower(float power_pct) {
    if (!initialized_) return false;
    
    // Clamp to safety limits
    power_pct = constrain(power_pct, 0.0f, (float)max_power_pct_);
    
    if (state_.fault) {
        return false;
    }
    
    state_.power_pct = power_pct;
    state_.enabled = (power_pct > 0.0f);
    
    applyDuty(power_to_duty(power_pct));
    
    return true;
}

void MosfetActuator::emergencyStop() {
    if (!initialized_) return;
    
    // Immediate hardware cutoff
    ledcWrite(pwm_channel_, 0);
    
    state_.enabled = false;
    state_.power_pct = 0.0f;
    state_.fault = true;
    state_.fault_message = "Emergency stop";
    
    Serial.println("[MosfetActuator] EMERGENCY STOP - Heater PWM forced to 0%");
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
    
    Serial.printf("[MosfetActuator] PID configured: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", kp, ki, kd);
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