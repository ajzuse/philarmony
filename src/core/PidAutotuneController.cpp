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

#include "PidAutotuneController.hpp"

#include <Arduino.h>
#include <math.h>

namespace filament_dryer {

PidAutotuneController::PidAutotuneController() {}

PidAutotuneController::~PidAutotuneController() {
    stopCalibration();
}

bool PidAutotuneController::startCalibration(const Config& config,
                                             ProgressCallback progress_cb,
                                             CompleteCallback complete_cb) {
    if (running_) {
        return false;
    }

    reset();
    config_ = config;
    progress_cb_ = progress_cb;
    complete_cb_ = complete_cb;
    running_ = true;
    state_ = HEATING_UP;
    cycle_start_time_ = millis();
    peak_temp_ = -1000.0f;
    valley_temp_ = 1000.0f;

    if (heater_cb_) {
        heater_cb_(config_.pwm_step);
    }

    return true;
}

void PidAutotuneController::stopCalibration() {
    running_ = false;
    state_ = IDLE;
    if (heater_cb_) {
        heater_cb_(0.0f);
    }
}

void PidAutotuneController::update(float current_temp, float heater_pwm_pct) {
    (void)heater_pwm_pct;

    if (!running_) {
        return;
    }

    checkSafety(current_temp);
    if (!running_) {
        return;
    }

    peak_temp_ = max(peak_temp_, current_temp);
    valley_temp_ = min(valley_temp_, current_temp);

    const float tolerance = 0.5f;
    const uint32_t elapsed_ms = millis() - cycle_start_time_;

    if (state_ == HEATING_UP && current_temp >= (config_.target_temp + tolerance)) {
        state_ = COOLING_DOWN;
        heating_phase_ = false;
        last_peak_time_ = millis();
        if (heater_cb_) {
            heater_cb_(0.0f);
        }
    } else if (state_ == COOLING_DOWN && current_temp <= (config_.target_temp - tolerance)) {
        cycles_completed_++;
        last_valley_time_ = millis();
        if (last_peak_time_ > 0 && last_valley_time_ > last_peak_time_) {
            oscillation_period_ = (last_valley_time_ - last_peak_time_) / 1000.0f;
        }
        oscillation_amplitude_ = max(0.1f, (peak_temp_ - valley_temp_) / 2.0f);

        if (cycles_completed_ >= config_.max_cycles) {
            calculatePid();
            return;
        }

        state_ = HEATING_UP;
        heating_phase_ = true;
        cycle_start_time_ = millis();
        peak_temp_ = current_temp;
        valley_temp_ = current_temp;
        if (heater_cb_) {
            heater_cb_(config_.pwm_step);
        }
    } else if (elapsed_ms > static_cast<uint32_t>(config_.cycle_timeout_sec) * 1000UL) {
        result_.success = false;
        result_.error = "Auto-tune timeout";
        running_ = false;
        state_ = ERROR;
        if (heater_cb_) {
            heater_cb_(0.0f);
        }
        if (complete_cb_) {
            complete_cb_(result_);
        }
        return;
    }

    if (progress_cb_) {
        progress_cb_(cycles_completed_, config_.max_cycles, current_temp,
                     result_.kp, result_.ki, result_.kd, false);
    }
}

void PidAutotuneController::reset() {
    state_ = IDLE;
    running_ = false;
    result_ = Result{};
    cycle_start_temp_ = 0.0f;
    peak_temp_ = 0.0f;
    valley_temp_ = 0.0f;
    cycle_start_time_ = 0;
    last_peak_time_ = 0;
    last_valley_time_ = 0;
    heating_phase_ = true;
    cycles_completed_ = 0;
    oscillation_period_ = 0.0f;
    oscillation_amplitude_ = 0.0f;
}

void PidAutotuneController::transitionTo(State new_state) {
    state_ = new_state;
}

void PidAutotuneController::checkSafety(float current_temp) {
    if (isnan(current_temp) || current_temp >= config_.max_temp) {
        result_.success = false;
        result_.error = isnan(current_temp) ? "Invalid temperature sample" : "Temperature exceeded limit";
        running_ = false;
        state_ = ERROR;
        if (heater_cb_) {
            heater_cb_(0.0f);
        }
        if (complete_cb_) {
            complete_cb_(result_);
        }
    }
}

void PidAutotuneController::calculatePid() {
    const float amplitude = max(0.1f, oscillation_amplitude_);
    const float period = max(1.0f, oscillation_period_);
    const float ku = (4.0f * max(1.0f, config_.pwm_step)) / (PI * amplitude);

    result_.kp = 0.6f * ku;
    result_.ki = (2.0f * result_.kp) / period;
    result_.kd = (result_.kp * period) / 8.0f;
    result_.success = true;
    result_.error = "";

    running_ = false;
    state_ = COMPLETE;
    if (heater_cb_) {
        heater_cb_(0.0f);
    }
    if (progress_cb_) {
        progress_cb_(cycles_completed_, config_.max_cycles, config_.target_temp,
                     result_.kp, result_.ki, result_.kd, true);
    }
    if (complete_cb_) {
        complete_cb_(result_);
    }
}

}  // namespace filament_dryer
