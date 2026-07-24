/**
 * PidAutotuneController - Implementation
 * Ziegler-Nichols Relay Auto-Tuning for heater PID calibration
 */
#include "PidAutotuneController.hpp"

namespace filament_dryer {

PidAutotuneController::PidAutotuneController() {}

PidAutotuneController::~PidAutotuneController() {
    stopCalibration();
}

bool PidAutotuneController::startCalibration(const Config& config,
                                             ProgressCallback progress_cb,
                                             CompleteCallback complete_cb) {
    if (running_) return false;
    
    config_ = config;
    progress_cb_ = progress_cb;
    complete_cb_ = complete_cb;
    running_ = true;
    
    result_ = {false, 0.0f, 0.0f, 0.0f, ""};
    
    state_ = HEATING_UP;
    cycle_start_temp_ = 0.0f;
    cycles_completed_ = 0;
    cycle_start_time_ = millis();
    heating_phase_ = true;
    peak_temp_ = -1000.0f;
    valley_temp_ = 1000.0f;
    oscillation_period_ = 0.0f;
    oscillation_amplitude_ = 0.0f;
    last_peak_time_ = 0;
    last_valley_time_ = 0;
    
    Serial.printf("[PidAutotune] Started: target=%.1fC, max_cycles=%d\n",
                  config_.target_temp, config_.max_cycles);
    
    return true;
}

void PidAutotuneController::stopCalibration() {
    running_ = false;
    state_ = IDLE;
    if (heater_cb_) heater_cb_(0.0f);
}

void PidAutotuneController::update(float current_temp, float heater_pwm_pct) {
    if (!running_) return;
    
    checkSafety(current_temp);
    
    if (state_ == ERROR) return;
    
    switch (state_) {
        case HEATING_UP:
            if (heater_cb_) heater_cb_(config_.pwm_step);
            
            if (current_temp >= config_.target_temp) {
                transitionTo(MEASURING_OSCILLATION);
            }
            break;
            
        case COOLING_DOWN:
            if (heater_cb_) heater_cb_(0.0f);
            
            if (current_temp <= config_.target_temp - 2.0f) {
                transitionTo(HEATING_UP);
            }
            break;
            
        case MEASURING_OSCILLATION: {
            // Relay control: toggle heater at target temp crossings
            if (heating_phase_) {
                if (current_temp >= config_.target_temp) {
                    heating_phase_ = false;
                    peak_temp_ = max(peak_temp_, current_temp);
                    last_peak_time_ = millis();
                    if (heater_cb_) heater_cb_(0.0f);
                }
            } else {
                if (current_temp <= config_.target_temp) {
                    heating_phase_ = true;
                    valley_temp_ = min(valley_temp_, current_temp);
                    last_valley_time_ = millis();
                    cycles_completed_++;
                    
                    if (cycles_completed_ > 1 && last_peak_time_ > 0 && last_valley_time_ > 0) {
                        float period = (last_peak_time_ - last_valley_time_) / 1000.0f * 2.0f; // Full period
                        oscillation_period_ = period;
                        oscillation_amplitude_ = peak_temp_ - valley_temp_;
                    }
                    
                    if (cycles_completed_ >= config_.max_cycles) {
                        transitionTo(CALCULATING);
                    }
                }
            }
            
            if (heating_phase_ && heater_cb_) heater_cb_(config_.pwm_step);
            else if (!heating_phase_ && heater_cb_) heater_cb_(0.0f);
            break;
        }
        
        case CALCULATING:
            calculatePid();
            break;
            
        case COMPLETE:
            if (complete_cb_) complete_cb_(result_);
            running_ = false;
            break;
            
        case ERROR:
            running_ = false;
            break;
            
        default:
            break;
    }
}

void PidAutotuneController::transitionTo(State new_state) {
    state_ = new_state;
    cycle_start_time_ = millis();
    
    if (progress_cb_ && new_state == MEASURING_OSCILLATION) {
        progress_cb_(cycles_completed_, config_.max_cycles, 0.0f, 0.0f, 0.0f, 0.0f, false);
    }
}

void PidAutotuneController::checkSafety(float current_temp) {
    if (current_temp >= config_.max_temp) {
        result_ = {false, 0, 0, 0, "Temperature exceeded safety limit: " + String(config_.max_temp) + "C"};
        state_ = ERROR;
        if (heater_cb_) heater_cb_(0.0f);
    }
}

void PidAutotuneController::calculatePid() {
    if (oscillation_period_ <= 0 || oscillation_amplitude_ <= 0) {
        result_ = {false, 0, 0, 0, "Insufficient oscillation data"};
        state_ = ERROR;
        return;
    }
    
    // Ziegler-Nichols relay tuning formulas
    // Ku = 4 * d / (pi * a)  where d = PWM step (0-1), a = amplitude
    float d = config_.pwm_step / 100.0f;
    float a = oscillation_amplitude_ / 2.0f; // Half amplitude
    
    if (a <= 0.01f) {
        result_ = {false, 0, 0, 0, "Oscillation amplitude too small"};
        state_ = ERROR;
        return;
    }
    
    float Ku = (4.0f * d) / (PI * a);
    float Tu = oscillation_period_;
    
    // Ziegler-Nichols PID tuning rules
    result_.kp = 0.6f * Ku;
    result_.ki = 1.2f * Ku / Tu;
    result_.kd = 0.075f * Ku * Tu;
    result_.success = true;
    result_.error = "";
    
    Serial.printf("[PidAutotune] Calculated: Kp=%.2f, Ki=%.2f, Kd=%.2f (Ku=%.2f, Tu=%.1fs)\n",
                  result_.kp, result_.ki, result_.kd, Ku, Tu);
    
    if (progress_cb_) {
        progress_cb_(config_.max_cycles, config_.max_cycles, 0.0f, 
                     result_.kp, result_.ki, result_.kd, true);
    }
    
    state_ = COMPLETE;
}

void PidAutotuneController::reset() {
    state_ = IDLE;
    running_ = false;
    result_ = {false, 0, 0, 0, ""};
}

} // namespace filament_dryer