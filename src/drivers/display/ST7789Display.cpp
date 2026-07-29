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
 * ST7789 TFT Display Driver - Implementation
 * LilyGo T-Display V1.1 (135x240), TTGO, T-QT
 * Uses LovyanGFX for hardware-accelerated SPI DMA rendering
 */
#include "ST7789Display.hpp"

namespace filament_dryer {

// LGFX configuration for LilyGo T-Display V1.1
class LGFX_TDisplay : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel;
    lgfx::Bus_SPI _bus;
    lgfx::Light_PWM _light;

public:
    LGFX_TDisplay(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc, int8_t rst, int8_t bl, uint16_t w, uint16_t h, uint8_t rot) {
        {
            auto cfg = _bus.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 80000000;
            cfg.freq_read = 20000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = sclk;
            cfg.pin_mosi = mosi;
            cfg.pin_miso = -1;
            cfg.pin_dc = dc;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        
        {
            auto cfg = _panel.config();
            cfg.pin_cs = cs;
            cfg.pin_rst = rst;
            cfg.pin_busy = -1;
            cfg.memory_width = w;
            cfg.memory_height = h;
            cfg.panel_width = w;
            cfg.panel_height = h;
            cfg.offset_x = rot == 1 ? 52 : (rot == 3 ? 53 : 0);
            cfg.offset_y = rot == 1 ? 40 : (rot == 3 ? 40 : 0);
            cfg.readable = true;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            _panel.config(cfg);
        }
        
        {
            auto cfg = _light.config();
            cfg.pin_bl = bl;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;
            _light.config(cfg);
            _panel.setLight(&_light);
        }
        
        setPanel(&_panel);
    }
};

ST7789Display::ST7789Display() {}

ST7789Display::~ST7789Display() {
    delete display_;
}

bool ST7789Display::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    mosi_pin_ = config["spi_mosi"] | 19;
    sclk_pin_ = config["spi_sclk"] | 18;
    cs_pin_ = config["spi_cs"] | 5;
    dc_pin_ = config["dc_pin"] | 16;
    rst_pin_ = config["rst_pin"] | 23;
    bl_pin_ = config["backlight_pin"] | 4;
    width_ = config["width"] | 135;
    height_ = config["height"] | 240;
    rotation_ = config["rotation"] | 1;
    
    metrics_.width = width_;
    metrics_.height = height_;
    metrics_.rotation = rotation_;
    metrics_.driver_name = "ST7789";
    
    display_ = new LGFX_TDisplay(mosi_pin_, sclk_pin_, cs_pin_, dc_pin_, rst_pin_, bl_pin_, width_, height_, rotation_);
    
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

void ST7789Display::clear() {
    if (display_) {
        display_->fillScreen(TFT_BLACK);
    }
}

bool ST7789Display::update(const JsonObject& status_fields) {
    if (!initialized_) return false;
    
    display_->fillScreen(TFT_BLACK);
    renderStatus(status_fields);
    return true;
}

void ST7789Display::showError(const String& message) {
    if (!display_) return;
    
    display_->fillScreen(TFT_RED);
    display_->setTextColor(TFT_WHITE, TFT_RED);
    display_->setTextSize(2);
    display_->setCursor(10, 50);
    display_->print("ERROR");
    display_->setTextSize(1);
    display_->setCursor(10, 80);
    display_->print(message);
}

void ST7789Display::showBootScreen(const String& firmware_version) {
    if (!display_) return;
    
    display_->fillScreen(TFT_BLACK);
    display_->setTextColor(TFT_CYAN, TFT_BLACK);
    display_->setTextSize(3);
    display_->setCursor(20, 60);
    display_->print("Filament");
    display_->setCursor(20, 100);
    display_->print("Dryer");
    display_->setTextSize(1);
    display_->setTextColor(TFT_WHITE, TFT_BLACK);
    display_->setCursor(20, 150);
    display_->print("v");
    display_->print(firmware_version);
}

DisplayMetrics ST7789Display::getMetrics() const {
    return metrics_;
}

bool ST7789Display::isConnected() {
    return initialized_ && display_ != nullptr;
}

void ST7789Display::setBrightness(uint8_t brightness) {
    current_brightness_ = brightness;
    if (display_) {
        display_->setBrightness(brightness);
    }
}

void ST7789Display::sleep() {
    if (display_) {
        display_->sleep();
    }
}

void ST7789Display::wake() {
    if (display_) {
        display_->wakeup();
    }
}

void ST7789Display::renderStatus(const JsonObject& fields) {
    if (!display_) return;
    
    display_->fillScreen(TFT_BLACK);
    display_->setTextColor(TFT_WHITE, TFT_BLACK);
    display_->setTextSize(1);
    
    // Title bar
    display_->fillRect(0, 0, width_, 25, TFT_NAVY);
    display_->setTextColor(TFT_CYAN, TFT_NAVY);
    display_->setCursor(5, 5);
    display_->print("Filament Dryer v0.1");
    
    int y = 35;
    const int line_h = 28;
    const int label_w = 60;
    
    auto drawField = [&](const String& label, const String& value, uint16_t valColor = TFT_WHITE) {
        if (y + line_h > height_) return;
        display_->setTextColor(TFT_GRAY, TFT_BLACK);
        display_->setCursor(10, y);
        display_->print(label);
        display_->setTextColor(valColor, TFT_BLACK);
        display_->setCursor(10 + label_w, y);
        display_->print(value);
        y += line_h;
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
    
    // Heater PWM
    if (fields.containsKey("heater_power_pct")) {
        float pwm = fields["heater_power_pct"];
        drawField("Heater:", String(pwm, 0) + "%", pwm > 50 ? TFT_ORANGE : TFT_GREEN);
    }
    
    // Fan PWM
    if (fields.containsKey("exhaust_fan_power_pct")) {
        float pwm = fields["exhaust_fan_power_pct"];
        drawField("Fan:", String(pwm, 0) + "%", TFT_BLUE);
    }
    
    // Status
    if (fields.containsKey("status")) {
        uint16_t color = fields["status"].as<String>() == "drying" ? TFT_GREEN : 
                         fields["status"].as<String>() == "fault_stopped" ? TFT_RED : TFT_YELLOW;
        drawField("Status:", fields["status"].as<String>(), color);
    }
    
    // Elapsed time
    if (fields.containsKey("elapsed_time_sec")) {
        uint32_t elapsed = fields["elapsed_time_sec"];
        uint32_t h = elapsed / 3600;
        uint32_t m = (elapsed % 3600) / 60;
        drawField("Time:", String(h) + "h " + String(m) + "m", TFT_WHITE);
    }
}

} // namespace filament_dryer