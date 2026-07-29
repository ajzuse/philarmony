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
 * PIDControl - PID temperature control with anti-windup
 */
#pragma once

#include "IControlAlgorithm.hpp"

namespace filament_dryer {

struct PIDConfig {
    float kp = 0;
    float ki = 0;
    float kd = 0;
    float max_integral = 1000;
    float min_output = 0;
    float max_output = 100;
};

class PIDControl : public IControlAlgorithm {
public:
    PIDControl();
    PIDControl(const JsonObject& config);
    ~PIDControl() override;
    
    bool begin(const JsonObject& config) override;
    float compute(float target, float current, float dt) override;
    void reset() override;
    String getType() const override { return "pid"; }
    String getName() const override { return "PID Control"; }
    bool needsTuning() const override { return true; }
    JsonObject getParameters() override;
    void setParameters(const JsonObject& params) override;
    bool isInitialized() const override { return initialized_; }

private:
    PIDConfig config_;
    float integral_ = 0.0f;
    float last_error_ = 0.0f;
    float last_output_ = 0.0f;
    bool initialized_ = false;
    bool tuning_mode_ = false;
    
    float clampOutput(float output);
};

} // namespace filament_dryer