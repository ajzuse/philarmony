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
 * PWMFeedforwardControl - Feedforward Control with PWM
 * Uses base PWM + temperature coefficient for predictive control
 */
#pragma once

#include "IControlAlgorithm.hpp"

namespace filament_dryer {

class PWMFeedforwardControl : public IControlAlgorithm {
public:
    PWMFeedforwardControl();
    PWMFeedforwardControl(const JsonObject& config);
    ~PWMFeedforwardControl() override;
    
    bool begin(const JsonObject& config) override;
    float compute(float target_temp, float current_temp, float dt) override;
    void reset() override;
    String getType() const override { return "pwm_feedforward"; }
    String getName() const override { return "PWM Feedforward"; }
    bool needsTuning() const override { return false; }
    JsonObject getParameters() override;
    void setParameters(const JsonObject& params) override;
    bool isInitialized() const override { return initialized_; }

private:
    float base_pwm_ = 50.0f;        // Base PWM at target temp
    float temp_coeff_ = 2.5f;       // PWM change per degree C
    float max_pwm_ = 100.0f;
    float min_pwm_ = 0.0f;
    bool initialized_ = false;
};

} // namespace filament_dryer