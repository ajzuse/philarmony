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
 * NTC Thermistor Sensor Driver - ADC with Beta Formula
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class NTCSensor : public ISensorDriver {
public:
    NTCSensor();
    ~NTCSensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "ntc"; }
    String getName() const override { return "NTC Thermistor"; }
    bool isConnected() override;

private:
    int8_t gpio_pin_ = 34;
    int8_t adc_channel_ = 6;  // ADC1_CH6 for GPIO34
    float beta_coefficient_ = 3950.0f;
    float series_resistor_ = 10000.0f;
    float nominal_resistance_ = 10000.0f;
    float nominal_temperature_ = 25.0f;
    int adc_attenuation_ = 11;  // 11dB = 3.3V range
    int adc_width_ = 12;  // 12-bit
    bool initialized_ = false;
    SensorReading last_reading_;

    float calculateTemperature(uint32_t adc_value);
};

} // namespace filament_dryer