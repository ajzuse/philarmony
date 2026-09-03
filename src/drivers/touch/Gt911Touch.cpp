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

#include "Gt911Touch.hpp"

namespace filament_dryer {

bool Gt911Touch::begin(const JsonObject& config) {
    const int8_t sda_pin = config["sda_pin"] | 21;
    const int8_t scl_pin = config["scl_pin"] | 22;
    const uint8_t i2c_bus = config["i2c_bus"] | 0;
#if defined(ESP32)
    wire_ = i2c_bus == 0 ? &Wire : &Wire1;
#else
    (void)i2c_bus;
    wire_ = &Wire;
#endif
    wire_->begin(sda_pin, scl_pin);
    wire_->setClock(400000);

    if (config.containsKey("i2c_address")) {
        i2c_address_ = config["i2c_address"].as<uint8_t>();
        initialized_ = probe(i2c_address_);
    } else {
        initialized_ = probe(0x5D);
        if (!initialized_) {
            initialized_ = probe(0x14);
        }
    }

    if (!initialized_) {
        last_error_ = "GT911 not found";
    }
    return initialized_;
}

TouchPoint Gt911Touch::read() {
    TouchPoint point;
    point.timestamp_ms = millis();
    if (!initialized_) {
        return point;
    }

    uint8_t status = 0;
    if (!readRegisters(0x814E, &status, 1)) {
        return point;
    }
    const uint8_t touch_count = status & 0x0F;
    if ((status & 0x80) == 0 || touch_count == 0) {
        if (status & 0x80) {
            writeRegister(0x814E, 0);
        }
        return point;
    }

    uint8_t data[4] = {};
    if (readRegisters(0x8150, data, sizeof(data))) {
        point.pressed = true;
        point.x = static_cast<int16_t>(data[0] | (data[1] << 8));
        point.y = static_cast<int16_t>(data[2] | (data[3] << 8));
    }
    writeRegister(0x814E, 0);
    return point;
}

bool Gt911Touch::isConnected() {
    return initialized_ && probe(i2c_address_);
}

void Gt911Touch::setSensitivity(const String& level) {
    // GT911 sensitivity tuning is configuration-checksum dependent.
    sensitivity_ = (level == "low" || level == "high") ? level : String("medium");
}

bool Gt911Touch::probe(uint8_t address) {
    wire_->beginTransmission(address);
    if (wire_->endTransmission() != 0) {
        return false;
    }
    i2c_address_ = address;
    return true;
}

bool Gt911Touch::readRegisters(uint16_t reg, uint8_t* data, size_t length) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(static_cast<uint8_t>(reg >> 8));
    wire_->write(static_cast<uint8_t>(reg & 0xFF));
    if (wire_->endTransmission() != 0) {
        return false;
    }

    wire_->requestFrom(i2c_address_, length);
    size_t received = 0;
    while (wire_->available() && received < length) {
        data[received++] = static_cast<uint8_t>(wire_->read());
    }
    return received == length;
}

bool Gt911Touch::writeRegister(uint16_t reg, uint8_t value) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(static_cast<uint8_t>(reg >> 8));
    wire_->write(static_cast<uint8_t>(reg & 0xFF));
    wire_->write(value);
    return wire_->endTransmission() == 0;
}

} // namespace filament_dryer
