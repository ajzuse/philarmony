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
 * ILI9488 Large TFT Display Driver (3.5" / 4.0" 320x480 color TFT)
 * Uses LovyanGFX for hardware-accelerated SPI / 8-bit parallel rendering.
 * Note: ILI9488 uses 18-bit color natively; LovyanGFX handles the translation.
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <LovyanGFX.hpp>

namespace filament_dryer {

class ILI9488Display : public IDisplayDriver {
public:
    ILI9488Display();
    ~ILI9488Display() override;

    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "ili9488"; }
    String getName() const override { return "ILI9488 TFT"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    uint8_t getBrightness() const override { return current_brightness_; }
    void setRotation(uint8_t rotation) override;
    void pushRgb565(int16_t x, int16_t y, uint16_t w, uint16_t h,
                    const uint16_t* data) override;
    void sleep() override;
    void wake() override;

private:
    class LGFX_ILI9488 : public lgfx::LGFX_Device {
        lgfx::Panel_ILI9488 _panel;
        lgfx::Bus_SPI       _bus;
        lgfx::Light_PWM     _light;
    public:
        LGFX_ILI9488(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc,
                     int8_t rst, int8_t bl, uint16_t w, uint16_t h, uint8_t rot) {
            {
                auto cfg = _bus.config();
                cfg.spi_host    = SPI2_HOST;
                cfg.spi_mode    = 0;
                // ILI9488 max SPI ~40 MHz in practice due to parasitic capacitance
                cfg.freq_write  = 40000000;
                cfg.freq_read   = 16000000;
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
                cfg.offset_x    = 0;
                cfg.offset_y    = 0;
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

    LGFX_ILI9488* display_ = nullptr;
    bool initialized_ = false;
    DisplayMetrics metrics_;
    int8_t  mosi_pin_  = 19;
    int8_t  sclk_pin_  = 18;
    int8_t  cs_pin_    = 5;
    int8_t  dc_pin_    = 16;
    int8_t  rst_pin_   = 23;
    int8_t  bl_pin_    = 4;
    uint16_t width_    = 320;
    uint16_t height_   = 480;
    uint8_t rotation_  = 1;
    uint8_t current_brightness_ = 255;

    void renderStatus(const JsonObject& fields);
    void drawField(int x, int y, const String& label, const String& value, uint16_t color);
};

} // namespace filament_dryer
