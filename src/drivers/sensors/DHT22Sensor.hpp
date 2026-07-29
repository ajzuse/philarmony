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
 * DHT22 Digital Temperature & Humidity Sensor Driver
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class DHT22Sensor : public ISensorDriver {
public:
    DHT22Sensor();
    ~DHT22Sensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "dht22"; }
    String getName() const override { return "DHT22 1-Wire Temp+Humidity"; }
    bool isConnected() override;
    
private:
    int8_t gpio_pin_ = 4;
    bool initialized_ = false;
    SensorReading last_reading_;
    uint32_t last_read_time_ = 0;
    uint32_t powerup_ready_ms_ = 0;

    enum class ReadPhase { IDLE, START_LOW };
    ReadPhase read_phase_ = ReadPhase::IDLE;
    uint32_t start_low_ms_ = 0;
    
    // DHT22 timing
    static constexpr uint32_t MIN_READ_INTERVAL_MS = 2000;
    
    bool completeTransaction(uint8_t* data);
    float calculateTemperature(int16_t raw);
    float calculateHumidity(int16_t raw);
};

} // namespace filament_dryer