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
 * AHT20/AHT10 Sensor Driver - I2C Temperature + Humidity
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <Wire.h>

namespace filament_dryer {

class AHT20Sensor : public ISensorDriver {
public:
    AHT20Sensor();
    ~AHT20Sensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "aht20"; }
    String getName() const override { return "AHT20/AHT10 Temp+Humidity"; }
    bool isConnected() override;

private:
    TwoWire* wire_ = &Wire;
    uint8_t i2c_address_ = 0x38;
    int8_t sda_pin_ = 21;
    int8_t scl_pin_ = 22;
    uint8_t i2c_bus_ = 0;
    bool initialized_ = false;
    SensorReading last_reading_;
    uint32_t last_read_time_ = 0;

    enum class ReadPhase { IDLE, WAITING };
    ReadPhase read_phase_ = ReadPhase::IDLE;
    uint32_t measure_ready_ms_ = 0;

    static constexpr uint8_t CMD_INIT = 0xBE;
    static constexpr uint8_t CMD_MEASURE = 0xAC;
    static constexpr uint8_t CMD_SOFT_RESET = 0xBA;

    bool sendCommand(uint8_t cmd, const uint8_t* data = nullptr, size_t len = 0);
    bool readData(uint8_t* data, size_t len);
    bool readStatus(uint8_t& status);
    float calcTemperature(uint32_t raw);
    float calcHumidity(uint32_t raw);
};

} // namespace filament_dryer