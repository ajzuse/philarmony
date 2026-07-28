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
 * DS18B20 1-Wire Temperature Sensor Driver
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <OneWire.h>

namespace filament_dryer {

class DS18B20Sensor : public ISensorDriver {
public:
    DS18B20Sensor();
    ~DS18B20Sensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "ds18b20"; }
    String getName() const override { return "DS18B20 1-Wire Temperature"; }
    bool isConnected() override;
    
private:
    int8_t gpio_pin_ = 4;
    uint8_t resolution_ = 12; // 9-12 bits
    OneWire* onewire_ = nullptr;
    bool initialized_ = false;
    SensorReading last_reading_;
    uint8_t device_addr_[8] = {0};

    enum class ReadPhase { IDLE, CONVERTING };
    ReadPhase read_phase_ = ReadPhase::IDLE;
    uint32_t convert_ready_ms_ = 0;
    
    bool findDevice();
    bool readScratchpad(int16_t& raw_temp);
};

} // namespace filament_dryer