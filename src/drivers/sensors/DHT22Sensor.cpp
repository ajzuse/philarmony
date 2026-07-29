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
 * DHT22Sensor - Non-blocking start pulse; bitbang uses micros only
 */
#include "DHT22Sensor.hpp"

namespace filament_dryer {

DHT22Sensor::DHT22Sensor() {}

DHT22Sensor::~DHT22Sensor() {}

bool DHT22Sensor::begin(const JsonObject& config) {
    gpio_pin_ = config["gpio_pin"] | 4;

    pinMode(gpio_pin_, INPUT_PULLUP);
    powerup_ready_ms_ = millis() + 1000; // DHT22 needs ~1s after power-up
    initialized_ = true;
    return true;
}

SensorReading DHT22Sensor::read() {
    SensorReading reading = last_reading_;
    reading.timestamp = millis();

    if (!initialized_) {
        reading.valid = false;
        reading.error_message = "Not initialized";
        return reading;
    }

    if (millis() < powerup_ready_ms_) {
        return reading;
    }

    if (read_phase_ == ReadPhase::IDLE) {
        if (millis() - last_read_time_ < MIN_READ_INTERVAL_MS && last_reading_.valid) {
            return reading;
        }
        pinMode(gpio_pin_, OUTPUT);
        digitalWrite(gpio_pin_, LOW);
        start_low_ms_ = millis();
        read_phase_ = ReadPhase::START_LOW;
        return reading;
    }

    if (millis() - start_low_ms_ < 20) {
        return reading;
    }

    uint8_t data[5] = {0};
    read_phase_ = ReadPhase::IDLE;
    if (completeTransaction(data)) {
        uint8_t checksum = data[0] + data[1] + data[2] + data[3];
        if (checksum == data[4]) {
            int16_t humidity_raw = (data[0] << 8) | data[1];
            int16_t temp_raw = (data[2] << 8) | data[3];
            reading.temperature = calculateTemperature(temp_raw);
            reading.humidity = calculateHumidity(humidity_raw);
            reading.valid = true;
            reading.error_message = "";
            last_reading_ = reading;
            last_read_time_ = millis();
        } else {
            reading.valid = false;
            reading.error_message = "Checksum error";
        }
    } else {
        reading.valid = false;
        reading.error_message = "Communication timeout";
    }

    return reading;
}

bool DHT22Sensor::completeTransaction(uint8_t* data) {
    digitalWrite(gpio_pin_, HIGH);
    delayMicroseconds(40);
    pinMode(gpio_pin_, INPUT_PULLUP);

    uint32_t timeout = micros() + 100;
    while (digitalRead(gpio_pin_) == HIGH) {
        if (micros() > timeout) return false;
    }

    timeout = micros() + 100;
    while (digitalRead(gpio_pin_) == LOW) {
        if (micros() > timeout) return false;
    }

    timeout = micros() + 100;
    while (digitalRead(gpio_pin_) == HIGH) {
        if (micros() > timeout) return false;
    }

    for (int byte_idx = 0; byte_idx < 5; byte_idx++) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; bit++) {
            timeout = micros() + 100;
            while (digitalRead(gpio_pin_) == LOW) {
                if (micros() > timeout) return false;
            }

            uint32_t high_start = micros();
            timeout = high_start + 100;
            while (digitalRead(gpio_pin_) == HIGH) {
                if (micros() > timeout) return false;
            }

            if ((micros() - high_start) > 40) {
                byte |= (1 << (7 - bit));
            }
        }
        data[byte_idx] = byte;
    }
    return true;
}

float DHT22Sensor::calculateTemperature(int16_t raw) {
    float t = raw / 10.0f;
    if (raw & 0x8000) {
        t = -((raw & 0x7FFF) / 10.0f);
    }
    return t;
}

float DHT22Sensor::calculateHumidity(int16_t raw) {
    return raw / 10.0f;
}

bool DHT22Sensor::isConnected() {
    return initialized_;
}

} // namespace filament_dryer
