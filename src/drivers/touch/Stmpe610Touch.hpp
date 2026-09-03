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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * Stmpe610Touch - STMPE610 resistive touch controller (I2C or SPI)
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#ifndef UNIT_TEST
#include <SPI.h>
#include <Wire.h>
#endif

namespace filament_dryer {

class Stmpe610Touch : public ITouchDriver {
public:
    bool begin(const JsonObject& config) override;
    TouchPoint read() override;
    bool isConnected() override;
    String getType() const override { return "stmpe610"; }
    void setSensitivity(const String& level) override;

private:
    uint8_t i2c_address_ = 0x41;
    int8_t cs_pin_ = -1;
    int8_t irq_pin_ = -1;
    int8_t mosi_pin_ = -1;
    int8_t miso_pin_ = -1;
    int8_t sclk_pin_ = -1;
    bool use_spi_ = false;
    bool initialized_ = false;
    String sensitivity_ = "medium";
#ifndef UNIT_TEST
    TwoWire* wire_ = &Wire;
    SPIClass* spi_ = nullptr;
#endif
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t reg, uint8_t* data, size_t length);
    bool identify();
    uint16_t pressureThreshold() const;
};

}  // namespace filament_dryer
