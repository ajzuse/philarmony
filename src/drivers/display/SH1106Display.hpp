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
 * SH1106 OLED Display Driver (1.3" 128x64 I2C/SPI OLED)
 * Similar to SSD1306 but uses page-based addressing with a 132-column controller.
 * Uses Adafruit SH110X library.
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

namespace filament_dryer {

class SH1106Display : public IDisplayDriver {
public:
    SH1106Display();
    ~SH1106Display() override;

    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "sh1106"; }
    String getName() const override { return "SH1106 OLED"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    void sleep() override;
    void wake() override;

private:
    Adafruit_SH1106G* display_ = nullptr;
    bool initialized_ = false;
    DisplayMetrics metrics_;
    uint8_t i2c_address_ = 0x3C;
    int8_t sda_pin_       = 21;
    int8_t scl_pin_       = 22;
    int8_t rst_pin_       = -1;
    uint8_t current_brightness_ = 255;

    void renderStatus(const JsonObject& fields);
};

} // namespace filament_dryer
