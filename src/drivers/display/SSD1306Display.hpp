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