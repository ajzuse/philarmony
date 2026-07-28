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
 * GC9A01Display - Implementation
 * 1.28" 240x240 circular TFT with LovyanGFX SPI DMA.
 * Layout adapts to the round bezel by centering all text radially.
 */
#include "GC9A01Display.hpp"

namespace filament_dryer {

GC9A01Display::GC9A01Display() {}

GC9A01Display::~GC9A01Display() {
    delete display_;
}

bool GC9A01Display::begin(const JsonObject& config) {
    if (initialized_) return true;

    mosi_pin_ = config["spi_mosi"]      | 19;
    sclk_pin_ = config["spi_sclk"]      | 18;
    cs_pin_   = config["spi_cs"]        | 5;
    dc_pin_   = config["dc_pin"]        | 16;
    rst_pin_  = config["rst_pin"]       | 23;
    bl_pin_   = config["backlight_pin"] | 4;

    metrics_.width       = 240;
    metrics_.height      = 240;
    metrics_.rotation    = config["rotation"] | 0;
    metrics_.driver_name = "GC9A01";

    display_ = new LGFX_GC9A01(mosi_pin_, sclk_pin_, cs_pin_, dc_pin_, rst_pin_, bl_pin_);

    if (!display_->init()) {
        delete display_;
        display_ = nullptr;
        return false;
    }

    display_->setRotation(metrics_.rotation);
    display_->setBrightness(current_brightness_);
    display_->fillScreen(TFT_BLACK);

    initialized_ = true;
    return true;
}

void GC9A01Display::clear() {
    if (display_) display_->fillScreen(TFT_BLACK);
}

bool GC9A01Display::update(const JsonObject& status_fields) {
    if (!initialized_) return false;
    display_->fillScreen(TFT_BLACK);
    renderStatus(status_fields);
    return true;
}

void GC9A01Display::showError(const String& message) {
    if (!display_) return;
    display_->fillScreen(TFT_RED);
    display_->setTextColor(TFT_WHITE, TFT_RED);
    drawCentered(100, "ERROR", 2, TFT_WHITE);
    drawCentered(130, message, 1, TFT_WHITE);
}

void GC9A01Display::showBootScreen(const String& firmware_version) {
    if (!display_) return;
    display_->fillScreen(TFT_BLACK);

    // Draw decorative outer ring
    display_->drawCircle(120, 120, 115, TFT_CYAN);
    display_->drawCircle(120, 120, 113, TFT_NAVY);

    drawCentered(80,  "Filament", 2, TFT_CYAN);
    drawCentered(108, "Dryer",    2, TFT_CYAN);
    drawCentered(150, "v" + firmware_version, 1, TFT_WHITE);
    delay(2000);
}

DisplayMetrics GC9A01Display::getMetrics() const {
    return metrics_;
}

bool GC9A01Display::isConnected() {
    return initialized_ && display_ != nullptr;
}

void GC9A01Display::setBrightness(uint8_t brightness) {
    current_brightness_ = brightness;
    if (display_) display_->setBrightness(brightness);
}

void GC9A01Display::sleep() {
    if (display_) display_->sleep();
}

void GC9A01Display::wake() {
    if (display_) display_->wakeup();
}

void GC9A01Display::drawCentered(int16_t y, const String& text, uint8_t sz, uint16_t color) {
    display_->setTextSize(sz);
    display_->setTextColor(color, TFT_BLACK);
    int16_t tw = text.length() * 6 * sz;
    int16_t x  = (240 - tw) / 2;
    display_->setCursor(x, y);
    display_->print(text);
}

void GC9A01Display::renderStatus(const JsonObject& fields) {
    if (!display_) return;

    // Outer decorative ring
    display_->drawCircle(120, 120, 115, TFT_NAVY);

    // Status-driven ring color
    String status_str = fields["status"] | "ready";
    uint16_t ring_color = (status_str == "drying")       ? TFT_GREEN  :
                          (status_str == "fault_stopped") ? TFT_RED    :
                          (status_str == "hotspot")       ? TFT_YELLOW :
                                                            TFT_CYAN;
    display_->drawCircle(120, 120, 117, ring_color);
    display_->drawCircle(120, 120, 118, ring_color);

    // Large temperature readout in the center
    if (fields.containsKey("chamber_temp_c")) {
        float temp = fields["chamber_temp_c"];
        String t_str = String(temp, 1) + "C";
        display_->setTextSize(3);
        display_->setTextColor(TFT_WHITE, TFT_BLACK);
        int16_t tw = t_str.length() * 18;
        display_->setCursor((240 - tw) / 2, 80);
        display_->print(t_str);
    }

    // Target temp
    if (fields.containsKey("target_temp_c")) {
        float tgt = fields["target_temp_c"];
        drawCentered(120, ">" + String(tgt, 0) + "C", 2, TFT_CYAN);
    }

    // Humidity
    if (fields.containsKey("humidity_pct")) {
        float hum = fields["humidity_pct"];
        drawCentered(155, String(hum, 1) + "% RH", 1, TFT_CYAN);
    }

    // Heater power arc indicator (simplified: text)
    if (fields.containsKey("heater_power_pct")) {
        float pwr = fields["heater_power_pct"];
        uint16_t col = pwr > 70 ? TFT_ORANGE : (pwr > 30 ? TFT_YELLOW : TFT_GREEN);
        drawCentered(175, "HTR " + String(pwr, 0) + "%", 1, col);
    }

    // Status line at bottom
    drawCentered(198, status_str, 1, ring_color);
}

} // namespace filament_dryer
