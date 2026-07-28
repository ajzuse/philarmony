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
 * SSR Actuator Driver - Solid State Relay Control
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class SSRActuator : public IActuatorDriver {
public:
    SSRActuator();
    ~SSRActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "ssr"; }
    String getName() const override { return "Solid State Relay"; }
    bool isHealthy() const override { return initialized_ && !state_.fault && !overcurrent_latched_; }

    bool hasFeedback() const override { return current_sense_pin_ >= 0 || open_loop_detect_; }
    float getMeasuredPowerPct() const override { return measured_power_pct_; }
    bool checkOvercurrent() override;

private:
    int8_t gpio_pin_ = 25;
    uint32_t pwm_freq_ = 1000;
    uint8_t pwm_channel_ = 0;
    uint8_t pwm_resolution_ = 10;
    uint8_t max_power_pct_ = 100;
    int8_t current_sense_pin_ = -1;
    uint16_t overcurrent_adc_threshold_ = 3000;
    bool open_loop_detect_ = false;
    int8_t open_loop_sense_pin_ = -1;
    float inject_measured_power_pct_ = -1.0f;
    bool overcurrent_latched_ = false;
    float measured_power_pct_ = 0.0f;
    bool emergency_stopped_ = false;
    bool initialized_ = false;
    ActuatorState state_;
    
    void applyDuty(uint32_t duty);
    void refreshOpenLoopMeasurement();
};

} // namespace filament_dryer
