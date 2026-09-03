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
 * Gt911Touch - GT911 capacitive touch controller
 */
#pragma once

#include <Wire.h>

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class Gt911Touch : public ITouchDriver {
public:
    bool begin(const JsonObject& config) override;
    TouchPoint read() override;
    bool isConnected() override;
    String getType() const override { return "gt911"; }
    void setSensitivity(const String& level) override;

private:
    TwoWire* wire_ = &Wire;
    uint8_t i2c_address_ = 0x5D;
    bool initialized_ = false;
    String sensitivity_ = "medium";

    bool probe(uint8_t address);
    bool readRegisters(uint16_t reg, uint8_t* data, size_t length);
    bool writeRegister(uint16_t reg, uint8_t value);
};

} // namespace filament_dryer
