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
 * ServoActuator - Servo Motor Driver
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class ServoActuator : public IActuatorDriver {
public:
    ServoActuator();
    ~ServoActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "servo"; }
    String getName() const override { return "Servo Motor"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }

private:
    int8_t pwm_pin_ = -1;
    int8_t enable_pin_ = -1;
    uint32_t pwm_freq_ = 50;
    uint8_t pwm_channel_ = 0;
    uint8_t pwm_resolution_ = 12;
    uint16_t min_pulse_ = 500;
    uint16_t max_pulse_ = 2500;
    float min_angle_ = 0;
    float max_angle_ = 180;
    bool initialized_ = false;
    ActuatorState state_;

    uint16_t angleToPulse(float angle) const;
    void applyPulse(uint16_t pulse_us);
};

} // namespace filament_dryer