/**
 * SH1106 OLED Display Driver (1.3" 128x64 I2C/SPI OLED)
 * Similar to SSD1306 but uses page-based addressing with a 132-column controller.
 * Uses Adafruit SH110X library.
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

namespace filament_dryer {

class SH1106Display : public IDisplayDriver {
public:
    SH1106Display();
    ~SH1106Display() override;

    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "sh1106"; }
    String getName() const override { return "SH1106 OLED"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    void sleep() override;
    void wake() override;

private:
    Adafruit_SH1106G* display_ = nullptr;
    bool initialized_ = false;
    DisplayMetrics metrics_;
    uint8_t i2c_address_ = 0x3C;
    int8_t sda_pin_       = 21;
    int8_t scl_pin_       = 22;
    int8_t rst_pin_       = -1;
    uint8_t current_brightness_ = 255;

    void renderStatus(const JsonObject& fields);
};

} // namespace filament_dryer
