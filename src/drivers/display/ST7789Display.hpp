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
 * ST7789 TFT Display Driver (LilyGo T-Display V1.1, TTGO, T-QT)
 * Uses LovyanGFX for hardware-accelerated rendering
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <LovyanGFX.hpp>

namespace filament_dryer {

class ST7789Display : public IDisplayDriver {
public:
    ST7789Display();
    ~ST7789Display() override;
    
    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "st7789"; }
    String getName() const override { return "ST7789 TFT"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    uint8_t getBrightness() const override { return current_brightness_; }
    void setRotation(uint8_t rotation) override;
    void pushRgb565(int16_t x, int16_t y, uint16_t w, uint16_t h,
                    const uint16_t* data) override;
    void sleep() override;
    void wake() override;

private:
    lgfx::LGFX_Device* display_ = nullptr;
    bool initialized_ = false;
    DisplayMetrics metrics_;
    int8_t mosi_pin_ = 19;
    int8_t sclk_pin_ = 18;
    int8_t cs_pin_ = 5;
    int8_t dc_pin_ = 16;
    int8_t rst_pin_ = 23;
    int8_t bl_pin_ = 4;
    uint16_t width_ = 135;
    uint16_t height_ = 240;
    uint16_t rotation_ = 1;
    uint8_t current_brightness_ = 255;
    
    void renderStatus(const JsonObject& fields);
    void drawLabelValue(int x, int y, const String& label, const String& value, uint16_t color = TFT_WHITE);
};

} // namespace filament_dryer