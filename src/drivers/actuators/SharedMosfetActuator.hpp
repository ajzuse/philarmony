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
 * SharedMosfetActuator - Heater and Fan sharing same MOSFET output
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class SharedMosfetActuator : public IActuatorDriver {
public:
    SharedMosfetActuator();
    ~SharedMosfetActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "shared_mosfet"; }
    String getName() const override { return "Shared MOSFET (Heater+Fan)"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }

    // Separate control for heater and fan
    bool setHeaterPower(float power_pct);
    bool setFanPower(float power_pct);

private:
    int8_t pwm_pin_ = -1;
    uint32_t pwm_freq_ = 1000;
    uint8_t pwm_channel_ = 0;
    uint8_t pwm_resolution_ = 10;
    uint8_t max_power_pct_ = 100;
    bool initialized_ = false;
    ActuatorState state_;
    float heater_power_ = 0;
    float fan_power_ = 0;

    void applyPWM(float power_pct);
};

} // namespace filament_dryer