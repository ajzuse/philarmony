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
 * ILI9341Display - Implementation
 * ESP32-2432S028 CYD (240x320)
 */
#include "ILI9341Display.hpp"

namespace filament_dryer {

ILI9341Display::ILI9341Display() {}

ILI9341Display::~ILI9341Display() {
    delete display_;
}

bool ILI9341Display::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    mosi_pin_ = config["spi_mosi"] | 13;
    sclk_pin_ = config["spi_sclk"] | 14;
    cs_pin_ = config["spi_cs"] | 15;
    dc_pin_ = config["dc_pin"] | 2;
    rst_pin_ = config["rst_pin"] | -1;
    bl_pin_ = config["backlight_pin"] | 21;
    width_ = config["width"] | 240;
    height_ = config["height"] | 320;
    rotation_ = config["rotation"] | 1;
    
    metrics_.width = width_;
    metrics_.height = height_;
    metrics_.rotation = rotation_;
    metrics_.driver_name = "ILI9341";
    
    display_ = new LGFX_CYD(mosi_pin_, sclk_pin_, cs_pin_, dc_pin_, rst_pin_, bl_pin_, width_, height_, rotation_);
    
    if (!display_->init()) {
        return false;
    }
    
    display_->setRotation(rotation_);
    display_->setBrightness(current_brightness_);
    display_->fillScreen(TFT_BLACK);
    display_->setTextColor(TFT_WHITE, TFT_BLACK);
    display_->setTextSize(1);
    
    initialized_ = true;
    return true;
}

void ILI9341Display::clear() {
    if (display_) {
        display_->fillScreen(TFT_BLACK);
    }
}

bool ILI9341Display::update(const JsonObject& status_fields) {
    if (!initialized_) return false;
    
    display_->fillScreen(TFT_BLACK);
    renderStatus(status_fields);
    return true;
}

void ILI9341Display::showError(const String& message) {
    if (!display_) return;
    
    display_->fillScreen(TFT_RED);
    display_->setTextColor(TFT_WHITE, TFT_RED);
    display_->setTextSize(2);
    display_->setCursor(10, 80);
    display_->print("ERROR");
    display_->setTextSize(1);
    display_->setCursor(10, 140);
    display_->print(message);
}

void ILI9341Display::showBootScreen(const String& firmware_version) {
    if (!display_) return;
    
    display_->fillScreen(TFT_BLACK);
    display_->setTextColor(TFT_CYAN, TFT_BLACK);
    display_->setTextSize(3);
    display_->setCursor(20, 80);
    display_->print("Filament");
    display_->setCursor(20, 140);
    display_->print("Dryer");
    display_->setTextSize(1);
    display_->setTextColor(TFT_WHITE, TFT_BLACK);
    display_->setCursor(20, 200);
    display_->print("v");
    display_->print(firmware_version);
    delay(2000);
}

DisplayMetrics ILI9341Display::getMetrics() const {
    return metrics_;
}

bool ILI9341Display::isConnected() {
    return initialized_ && display_ != nullptr;
}

void ILI9341Display::setBrightness(uint8_t brightness) {
    current_brightness_ = brightness;
    if (display_) {
        display_->setBrightness(brightness);
    }
}

void ILI9341Display::sleep() {
    if (display_) {
        display_->sleep();
    }
}

void ILI9341Display::wake() {
    if (display_) {
        display_->wakeup();
    }
}

void ILI9341Display::renderStatus(const JsonObject& fields) {
    if (!display_) return;
    
    display_->fillScreen(TFT_BLACK);
    display_->setTextColor(TFT_WHITE, TFT_BLACK);
    display_->setTextSize(1);
    
    // Title bar
    display_->fillRect(0, 0, width_, 30, TFT_NAVY);
    display_->setTextColor(TFT_CYAN, TFT_NAVY);
    display_->setCursor(5, 5);
    display_->print("Filament Dryer v0.1");
    
    int y = 40;
    const int line_h = 32;
    
    auto drawField = [&](const String& label, const String& value, uint16_t color = TFT_WHITE) {
        if (y + 28 > height_) return;
        display_->setTextColor(TFT_GRAY, TFT_BLACK);
        display_->setCursor(15, y);
        display_->print(label);
        display_->setTextColor(TFT_WHITE, TFT_BLACK);
        display_->setCursor(100, y);
        display_->print(value);
        y += 28;
    };
    
    // Temperature
    if (fields.containsKey("chamber_temp_c")) {
        float temp = fields["chamber_temp_c"];
        float target = fields["target_temp_c"] | 0;
        drawField("Temp:", String(temp, 1) + " / " + String(target, 0) + "C", 
                  temp > target ? TFT_RED : TFT_GREEN);
    }
    
    // Humidity
    if (fields.containsKey("humidity_pct")) {
        float hum = fields["humidity_pct"];
        float target = fields["target_humidity_pct"] | 0;
        drawField("Hum:", String(hum, 1) + " / " + String(target, 0) + "%", TFT_CYAN);
    }
    
    // Heater
    if (fields.containsKey("heater_power_pct")) {
        float pwm = fields["heater_power_pct"];
        drawField("Heater:", String(pwm, 0) + "%", pwm > 50 ? TFT_ORANGE : TFT_WHITE);
    }
    
    // Fan
    if (fields.containsKey("exhaust_fan_power_pct")) {
        float pwm = fields["exhaust_fan_power_pct"];
        drawField("Fan:", String(pwm, 0) + "%", TFT_CYAN);
    }
    
    // Status
    if (fields.containsKey("status")) {
        drawField("Status:", fields["status"].as<String>(), TFT_GREEN);
    }
    
    // Elapsed time
    if (fields.containsKey("elapsed_time_sec")) {
        uint32_t elapsed = fields["elapsed_time_sec"];
        uint32_t h = elapsed / 3600;
        uint32_t m = (elapsed % 3600) / 60;
        drawField("Time:", String(h) + "h" + String(m) + "m", TFT_WHITE);
    }
}

void ILI9341Display::drawField(int x, int y, const String& label, const String& value, uint16_t color) {
    display_->setTextColor(TFT_GRAY, TFT_BLACK);
    display_->setCursor(x, y);
    display_->print(label);
    display_->setTextColor(color, TFT_BLACK);
    display_->setCursor(x + 80, y);
    display_->print(value);
}

} // namespace filament_dryer