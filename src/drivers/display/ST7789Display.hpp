/**
 * ST7789 TFT Display Driver (LilyGo T-Display V1.1, TTGO, T-QT)
 * Uses LovyanGFX for hardware-accelerated rendering
 */
#pragma once

#include "IDriverInterfaces.hpp"
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
    void sleep() override;
    void wake() override;

private:
    LGFX* display_ = nullptr;
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