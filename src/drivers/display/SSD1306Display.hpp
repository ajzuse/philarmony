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
 * SSD1306 OLED Display Driver
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace filament_dryer {

class SSD1306Display : public IDisplayDriver {
public:
    SSD1306Display();
    ~SSD1306Display() override;
    
    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "ssd1306"; }
    String getName() const override { return "SSD1306 OLED"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    void sleep() override;
    void wake() override;

private:
    Adafruit_SSD1306* display_ = nullptr;
    bool initialized_ = false;
    DisplayMetrics metrics_;
    uint8_t i2c_address_ = 0x3C;
    int8_t sda_pin_ = 21;
    int8_t scl_pin_ = 22;
    int8_t rst_pin_ = -1;
    uint8_t current_brightness_ = 255;
    
    void renderStatus(const JsonObject& fields);
    void drawField(int16_t x, int16_t y, const String& label, const String& value, bool highlight = false);
};

} // namespace filament_dryer