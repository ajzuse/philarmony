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

#include "Cst816sTouch.hpp"

namespace filament_dryer {

bool Cst816sTouch::begin(const JsonObject& config) {
    i2c_address_ = config["i2c_address"] | 0x15;
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
    initialized_ = isConnected();
    if (!initialized_) {
        last_error_ = "CST816S not found";
    }
    return initialized_;
}

TouchPoint Cst816sTouch::read() {
    TouchPoint point;
    point.timestamp_ms = millis();
    if (!initialized_) {
        return point;
    }

    uint8_t data[5] = {};
    if (!readRegisters(0x02, data, sizeof(data)) || data[0] == 0) {
        return point;
    }

    point.pressed = true;
    point.x = static_cast<int16_t>(((data[1] & 0x0F) << 8) | data[2]);
    point.y = static_cast<int16_t>(((data[3] & 0x0F) << 8) | data[4]);
    return point;
}

bool Cst816sTouch::isConnected() {
    wire_->beginTransmission(i2c_address_);
    return wire_->endTransmission() == 0;
}

void Cst816sTouch::setSensitivity(const String& level) {
    // CST816S sensitivity settings vary by firmware revision.
    sensitivity_ = (level == "low" || level == "high") ? level : String("medium");
}

bool Cst816sTouch::readRegisters(uint8_t reg, uint8_t* data, size_t length) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
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

} // namespace filament_dryer
