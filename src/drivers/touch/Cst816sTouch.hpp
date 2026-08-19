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
 * Cst816sTouch - CST816S capacitive touch controller
 */
#pragma once

#include <Wire.h>

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class Cst816sTouch : public ITouchDriver {
public:
    bool begin(const JsonObject& config) override;
    TouchPoint read() override;
    bool isConnected() override;
    String getType() const override { return "cst816s"; }
    void setSensitivity(const String& level) override;

private:
    TwoWire* wire_ = &Wire;
    uint8_t i2c_address_ = 0x15;
    bool initialized_ = false;
    String sensitivity_ = "medium";

    bool readRegisters(uint8_t reg, uint8_t* data, size_t length);
};

} // namespace filament_dryer
