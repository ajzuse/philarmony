/**
 * PidAutotuneController - Implementation
 * Ziegler-Nichols Relay Auto-Tuning for heater PID calibration
 */
#include "PidAutotuneController.hpp"
#include <Arduino.h>

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
    
    result_.success = false;
    result_.kp = 0.0f;
    result_.ki = 0.0f;
    result_.kd = 0.0f;
    result_.error = "";
    
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

} // namespace filament_dryer