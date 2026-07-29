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
 * BangBangControl - Hysteresis-based on/off control
 */
#pragma once

#include "IControlAlgorithm.hpp"

namespace filament_dryer {

struct BangBangConfig {
    float hysteresis = 1.0f;  // degrees C
    float min_cycle_time = 10.0f;  // seconds
};

class BangBangControl : public IControlAlgorithm {
public:
    BangBangControl();
    BangBangControl(const JsonObject& config);
    ~BangBangControl() override;
    
    bool begin(const JsonObject& config) override;
    float compute(float target, float current, float dt) override;
    void reset() override;
    String getType() const override { return "bang_bang"; }
    String getName() const override { return "Bang-Bang (Hysteresis)"; }
    bool needsTuning() const override { return true; }
    JsonObject getParameters() override;
    void setParameters(const JsonObject& params) override;
    bool isInitialized() const override { return initialized_; }

private:
    BangBangConfig config_;
    float last_output_ = 0.0f;
    uint32_t last_switch_time_ = 0;
    bool initialized_ = false;
    
    bool shouldSwitch(float target, float current);
};

} // namespace filament_dryer