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
 * SHT3x I2C Temperature & Humidity Sensor Driver (non-blocking read)
 */
#include "SHT3xSensor.hpp"

namespace filament_dryer {

SHT3xSensor::SHT3xSensor() {}

SHT3xSensor::~SHT3xSensor() {}

bool SHT3xSensor::begin(const JsonObject& config) {
    if (initialized_) return true;

    i2c_address_ = config["i2c_address"] | 0x44;
    i2c_bus_ = config["i2c_bus"] | 0;
    sda_pin_ = config["sda_pin"] | 21;
    scl_pin_ = config["scl_pin"] | 22;

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
    wire_->setClock(400000);

    if (!writeCommand(CMD_SOFT_RESET)) {
        return false;
    }
    // Soft-reset settle happens before control loop; use micros busy-wait briefly
    // only during begin (not in the 50Hz read path).
    const uint32_t start = micros();
    while (micros() - start < 10000) {
        yield();
    }

    initialized_ = true;
    return true;
}

SensorReading SHT3xSensor::read() {
    SensorReading reading = last_reading_;
    reading.timestamp = millis();

    if (!initialized_) {
        reading.valid = false;
        reading.error_message = "Not initialized";
        return reading;
    }

    if (read_phase_ == ReadPhase::IDLE) {
        if (!writeCommand(CMD_MEAS_HIGHREP)) {
            reading.valid = false;
            reading.error_message = "I2C write failed";
            return reading;
        }
        pending_cmd_ = CMD_MEAS_HIGHREP;
        measure_ready_ms_ = millis() + 15;
        read_phase_ = ReadPhase::WAITING;
        return reading; // return last valid while measuring
    }

    if (millis() < measure_ready_ms_) {
        return reading;
    }

    uint8_t data[6];
    if (!fetchMeasurement(data, 6)) {
        read_phase_ = ReadPhase::IDLE;
        reading.valid = false;
        reading.error_message = "I2C read failed";
        return reading;
    }
    read_phase_ = ReadPhase::IDLE;

    uint16_t temp_raw = (data[0] << 8) | data[1];
    if (crc8(data, 2) != data[2]) {
        reading.valid = false;
        reading.error_message = "Temperature CRC mismatch";
        return reading;
    }

    uint16_t hum_raw = (data[3] << 8) | data[4];
    if (crc8(data + 3, 2) != data[5]) {
        reading.valid = false;
        reading.error_message = "Humidity CRC mismatch";
        return reading;
    }

    reading.temperature = calculateTemperature(temp_raw);
    reading.humidity = calculateHumidity(hum_raw);
    reading.valid = true;
    reading.error_message = "";
    last_reading_ = reading;
    return reading;
}

bool SHT3xSensor::isConnected() {
    if (!initialized_) return false;
    uint8_t status[3];
    if (!writeCommand(CMD_READ_STATUS)) return false;
    wire_->requestFrom(i2c_address_, (uint8_t)3);
    if (wire_->available() < 3) return false;
    for (int i = 0; i < 3; i++) status[i] = wire_->read();
    (void)status;
    return true;
}

bool SHT3xSensor::writeCommand(uint16_t cmd) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(cmd >> 8);
    wire_->write(cmd & 0xFF);
    return wire_->endTransmission() == 0;
}

bool SHT3xSensor::fetchMeasurement(uint8_t* data, size_t len) {
    wire_->requestFrom(i2c_address_, len);
    if (wire_->available() < (int)len) return false;
    for (size_t i = 0; i < len; i++) {
        data[i] = wire_->read();
    }
    return true;
}

uint8_t SHT3xSensor::crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc <<= 1;
        }
    }
    return crc;
}

float SHT3xSensor::calculateTemperature(uint16_t raw) {
    return -45.0f + 175.0f * (float)raw / 65535.0f;
}

float SHT3xSensor::calculateHumidity(uint16_t raw) {
    return 100.0f * (float)raw / 65535.0f;
}

} // namespace filament_dryer
