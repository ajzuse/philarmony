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
 * ILI9341 TFT Display Driver (ESP32-2432S028 CYD)
 * Uses LovyanGFX for hardware-accelerated rendering
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <LovyanGFX.hpp>

namespace filament_dryer {

class ILI9341Display : public IDisplayDriver {
public:
    ILI9341Display();
    ~ILI9341Display() override;
    
    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "ili9341"; }
    String getName() const override { return "ILI9341 TFT"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    uint8_t getBrightness() const override { return current_brightness_; }
    void pushRgb565(int16_t x, int16_t y, uint16_t w, uint16_t h,
                    const uint16_t* data) override;
    void sleep() override;
    void wake() override;

private:
    class LGFX_CYD : public lgfx::LGFX_Device {
        lgfx::Panel_ILI9341 _panel;
        lgfx::Bus_SPI _bus;
        lgfx::Light_PWM _light;
        
    public:
        LGFX_CYD(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc, int8_t rst, int8_t bl, uint16_t w, uint16_t h, uint8_t rot) {
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
            
            auto pcfg = _panel.config();
            pcfg.pin_cs = cs;
            pcfg.pin_rst = rst;
            pcfg.pin_busy = -1;
            pcfg.memory_width = w;
            pcfg.memory_height = h;
            pcfg.panel_width = w;
            pcfg.panel_height = h;
            pcfg.offset_x = 0;
            pcfg.offset_y = 0;
            pcfg.readable = true;
            pcfg.invert = false;
            pcfg.rgb_order = false;
            pcfg.dlen_16bit = false;
            pcfg.bus_shared = false;
            _panel.config(pcfg);
            
            auto lcfg = _light.config();
            lcfg.pin_bl = bl;
            lcfg.invert = false;
            lcfg.freq = 44100;
            lcfg.pwm_channel = 7;
            _light.config(lcfg);
            _panel.setLight(&_light);
            
            setPanel(&_panel);
        }
    };
    
    LGFX_CYD* display_ = nullptr;
    bool initialized_ = false;
    DisplayMetrics metrics_;
    int8_t mosi_pin_ = 13;
    int8_t sclk_pin_ = 14;
    int8_t cs_pin_ = 15;
    int8_t dc_pin_ = 2;
    int8_t rst_pin_ = -1;
    int8_t bl_pin_ = 21;
    uint16_t width_ = 240;
    uint16_t height_ = 320;
    uint8_t rotation_ = 1;
    uint8_t current_brightness_ = 255;
    
    void renderStatus(const JsonObject& fields);
    void drawField(int x, int y, const String& label, const String& value, uint16_t color = TFT_WHITE);
};

} // namespace filament_dryer