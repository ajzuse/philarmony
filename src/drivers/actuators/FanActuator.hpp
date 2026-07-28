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
 * Fan Actuator Driver - Supports PWM, Digital, and Shared MOSFET modes
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class FanActuator : public IActuatorDriver {
public:
    enum class FanMode {
        SHARED_MOSFET,      // Fan shares MOSFET with heater
        INDEPENDENT_PWM,    // Dedicated PWM MOSFET
        INDEPENDENT_DIGITAL // Dedicated digital GPIO (on/off)
    };
    
    FanActuator();
    ~FanActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "fan_pwm"; }
    String getName() const override { return "Exhaust Fan"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }
    
private:
    int8_t gpio_pin_ = 26;
    uint32_t pwm_freq_ = 5000;
    uint8_t pwm_channel_ = 1;
    uint8_t pwm_resolution_ = 10;
    FanMode mode_ = FanMode::INDEPENDENT_PWM;
    int8_t shared_heater_pin_ = -1; // For shared MOSFET mode
    bool initialized_ = false;
    ActuatorState state_;
    
    FanMode parseFanMode(const String& mode_str);
    float power_to_duty(float power_pct) const;
    void applyDuty(uint32_t duty);
};

} // namespace filament_dryer