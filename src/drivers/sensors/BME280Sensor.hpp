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
 * BME280/BMP280 Sensor Driver - I2C/SPI Temperature + Pressure + Humidity
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <Wire.h>

namespace filament_dryer {

class BME280Sensor : public ISensorDriver {
public:
    BME280Sensor();
    ~BME280Sensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "bme280"; }
    String getName() const override { return "BME280/BMP280 Temp+Pressure+Humidity"; }
    bool isConnected() override;

private:
    TwoWire* wire_ = &Wire;
    uint8_t i2c_address_ = 0x76;
    uint8_t i2c_bus_ = 0;
    int8_t sda_pin_ = 21;
    int8_t scl_pin_ = 22;
    bool initialized_ = false;
    bool is_bmp280_ = false;  // Detect if BMP280 (no humidity)
    SensorReading last_reading_;
    
    // BME280 registers
    static constexpr uint8_t REG_ID = 0xD0;
    static constexpr uint8_t REG_CTRL_HUM = 0xF2;
    static constexpr uint8_t REG_CTRL_MEAS = 0xF4;
    static constexpr uint8_t REG_CONFIG = 0xF5;
    static constexpr uint8_t REG_PRESS_MSB = 0xF7;
    static constexpr uint8_t REG_TEMP_MSB = 0xFA;
    static constexpr uint8_t REG_HUM_MSB = 0xFD;
    static constexpr uint8_t REG_CALIB_START = 0x88;
    static constexpr uint8_t REG_HUM_CALIB = 0xE1;

    // Calibration data
    struct CalibrationData {
        uint16_t dig_T1;
        int16_t dig_T2, dig_T3;
        uint16_t dig_P1;
        int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
        uint8_t dig_H1;
        int16_t dig_H2;
        uint8_t dig_H3;
        int16_t dig_H4, dig_H5;
        int8_t dig_H6;
    } calib_;

    int32_t t_fine_ = 0;

    bool readCalibration();
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t reg, uint8_t* data, size_t len);
    float compensateTemperature(int32_t adc_T);
    float compensatePressure(int32_t adc_P);
    float compensateHumidity(int32_t adc_H);
};

} // namespace filament_dryer