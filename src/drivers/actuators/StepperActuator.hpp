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
 * StepperActuator - Stepper Motor Driver (A4988, DRV8825, TMC2209, etc.)
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class StepperActuator : public IActuatorDriver {
public:
    StepperActuator();
    ~StepperActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "stepper"; }
    String getName() const override { return "Stepper Motor"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }

    // Stepper-specific methods
    bool moveSteps(int32_t steps);
    bool setSpeed(float steps_per_sec);
    bool home();

private:
    int8_t step_pin_ = -1;
    int8_t dir_pin_ = -1;
    int8_t enable_pin_ = -1;
    int8_t ms1_pin_ = -1;
    int8_t ms2_pin_ = -1;
    int8_t ms3_pin_ = -1;
    uint32_t max_speed_ = 1000;
    float current_speed_ = 0;
    int32_t target_position_ = 0;
    int32_t current_position_ = 0;
    uint8_t microsteps_ = 16;
    bool initialized_ = false;
    ActuatorState state_;

    void setMicrostepping(uint8_t ms);
    void setDirection(bool cw);
    void stepPulse();
};

} // namespace filament_dryer