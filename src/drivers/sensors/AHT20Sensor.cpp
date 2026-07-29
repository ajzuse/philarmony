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
 * AHT20Sensor - Implementation
 * AHT20/AHT10 I2C Temperature + Humidity Sensor
 */
#include "AHT20Sensor.hpp"

namespace filament_dryer {

AHT20Sensor::AHT20Sensor() {}

AHT20Sensor::~AHT20Sensor() {}

bool AHT20Sensor::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    i2c_address_ = config["i2c_address"] | 0x38;
    sda_pin_ = config["sda_pin"] | 21;
    scl_pin_ = config["scl_pin"] | 22;
    i2c_bus_ = config["i2c_bus"] | 0;

    if (i2c_bus_ == 0) {
        wire_ = &Wire;
    } else {
        #if defined(ESP32)
        wire_ = &Wire1;
        #else
        wire_ = &Wire;
        #endif
    }

    wire_->begin(sda_pin_, scl_pin_);
    wire_->setClock(400000);  // 400kHz fast mode

    // Soft reset
    if (!sendCommand(CMD_SOFT_RESET)) {
        return false;
    }
    // Soft reset settle (begin only)
    const uint32_t reset_start = micros();
    while (micros() - reset_start < 20000) {
        yield();
    }

    // Initialize - normal mode
    uint8_t init_data[2] = {0x08, 0x00};  // Normal mode, calibration enabled
    if (!sendCommand(CMD_INIT, init_data, 2)) {
        return false;
    }
    const uint32_t init_start = micros();
    while (micros() - init_start < 100000) {
        yield();
    }

    // Wait for calibration bit (poll without delay())
    for (int i = 0; i < 50; i++) {
        uint8_t status;
        if (readStatus(status) && (status & 0x08)) break;
        const uint32_t w = micros();
        while (micros() - w < 2000) {
            yield();
        }
    }

    initialized_ = true;
    return true;
}

bool AHT20Sensor::sendCommand(uint8_t cmd, const uint8_t* data, size_t len) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(cmd);
    for (size_t i = 0; i < len; i++) {
        wire_->write(data[i]);
    }
    return wire_->endTransmission() == 0;
}

bool AHT20Sensor::readData(uint8_t* data, size_t len) {
    wire_->requestFrom(i2c_address_, len);
    size_t received = 0;
    uint32_t start = millis();
    while (received < len && millis() - start < 5) {
        if (wire_->available()) {
            data[received++] = wire_->read();
        }
    }
    return received == len;
}

bool AHT20Sensor::readStatus(uint8_t& status) {
    wire_->requestFrom(i2c_address_, 1);
    if (wire_->available()) {
        status = wire_->read();
        return true;
    }
    return false;
}

SensorReading AHT20Sensor::read() {
    SensorReading reading = last_reading_;
    reading.timestamp = millis();

    if (!initialized_) {
        reading.valid = false;
        reading.error_message = "Not initialized";
        return reading;
    }

    if (read_phase_ == ReadPhase::IDLE) {
        uint8_t cmd[2] = {0x33, 0x00};
        if (!sendCommand(CMD_MEASURE, cmd, 2)) {
            reading.valid = false;
            reading.error_message = "Failed to trigger measurement";
            return reading;
        }
        measure_ready_ms_ = millis() + 80;
        read_phase_ = ReadPhase::WAITING;
        return reading;
    }

    uint8_t status = 0xFF;
    if (readStatus(status) && (status & 0x80)) {
        // still busy
        if (millis() > measure_ready_ms_ + 50) {
            read_phase_ = ReadPhase::IDLE;
            reading.valid = false;
            reading.error_message = "Measurement timeout";
        }
        return reading;
    }

    if (millis() < measure_ready_ms_ && (status & 0x80)) {
        return reading;
    }

    uint8_t data[7];
    read_phase_ = ReadPhase::IDLE;
    if (!readData(data, 7)) {
        reading.valid = false;
        reading.error_message = "I2C read failed";
        return reading;
    }

    uint32_t raw_hum = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t raw_temp = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    reading.temperature = calcTemperature(raw_temp);
    reading.humidity = calcHumidity(raw_hum);
    reading.valid = true;
    reading.error_message = "";
    last_reading_ = reading;
    last_read_time_ = millis();
    return reading;
}

float AHT20Sensor::calcTemperature(uint32_t raw) {
    return (float)raw * 200.0f / 1048576.0f - 50.0f;
}

float AHT20Sensor::calcHumidity(uint32_t raw) {
    return (float)raw * 100.0f / 1048576.0f;
}

bool AHT20Sensor::isConnected() {
    if (!initialized_) return false;
    uint8_t status;
    return readStatus(status);
}

} // namespace filament_dryer