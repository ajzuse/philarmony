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
 * ST7735 TFT Display Driver (1.44" / 1.8" color TFT, 128x128 or 128x160)
 * Uses LovyanGFX for hardware-accelerated SPI DMA rendering.
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <LovyanGFX.hpp>

namespace filament_dryer {

class ST7735Display : public IDisplayDriver {
public:
    ST7735Display();
    ~ST7735Display() override;

    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "st7735"; }
    String getName() const override { return "ST7735 TFT"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    uint8_t getBrightness() const override { return current_brightness_; }
    void pushRgb565(int16_t x, int16_t y, uint16_t w, uint16_t h,
                    const uint16_t* data) override;
    void sleep() override;
    void wake() override;

private:
    // Nested LGFX device for ST7735
    class LGFX_ST7735 : public lgfx::LGFX_Device {
        lgfx::Panel_ST7735  _panel;
        lgfx::Bus_SPI       _bus;
        lgfx::Light_PWM     _light;
    public:
        LGFX_ST7735(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc,
                    int8_t rst, int8_t bl, uint16_t w, uint16_t h, uint8_t rot) {
            {
                auto cfg = _bus.config();
                cfg.spi_host    = SPI2_HOST;
                cfg.spi_mode    = 0;
                cfg.freq_write  = 27000000; // ST7735 max SPI ~27 MHz
                cfg.freq_read   = 14000000;
                cfg.spi_3wire   = true;
                cfg.use_lock    = true;
                cfg.dma_channel = SPI_DMA_CH_AUTO;
                cfg.pin_sclk    = sclk;
                cfg.pin_mosi    = mosi;
                cfg.pin_miso    = -1;
                cfg.pin_dc      = dc;
                _bus.config(cfg);
                _panel.setBus(&_bus);
            }
            {
                auto cfg = _panel.config();
                cfg.pin_cs      = cs;
                cfg.pin_rst     = rst;
                cfg.pin_busy    = -1;
                cfg.memory_width  = w;
                cfg.memory_height = h;
                cfg.panel_width   = w;
                cfg.panel_height  = h;
                // ST7735 128x160 has a 2-pixel x-offset in some variants
                cfg.offset_x    = (w == 128 && h == 160) ? 2 : 0;
                cfg.offset_y    = 1;
                cfg.readable    = false;
                cfg.invert      = false;
                cfg.rgb_order   = false;
                cfg.dlen_16bit  = false;
                cfg.bus_shared  = false;
                _panel.config(cfg);
            }
            if (bl >= 0) {
                auto cfg = _light.config();
                cfg.pin_bl      = bl;
                cfg.invert      = false;
                cfg.freq        = 44100;
                cfg.pwm_channel = 7;
                _light.config(cfg);
                _panel.setLight(&_light);
            }
            setPanel(&_panel);
        }
    };

    LGFX_ST7735* display_ = nullptr;
    bool initialized_ = false;
    DisplayMetrics metrics_;
    int8_t  mosi_pin_  = 19;
    int8_t  sclk_pin_  = 18;
    int8_t  cs_pin_    = 5;
    int8_t  dc_pin_    = 16;
    int8_t  rst_pin_   = 23;
    int8_t  bl_pin_    = 4;
    uint16_t width_    = 128;
    uint16_t height_   = 160;
    uint8_t rotation_  = 1;
    uint8_t current_brightness_ = 255;

    void renderStatus(const JsonObject& fields);
};

} // namespace filament_dryer
