/**
 * HD44780 Character LCD Display Driver (16x2 / 20x4 via PCF8574 I2C backpack)
 * Uses the LiquidCrystal_I2C library for compact I2C communication.
 * This is the classic 1602/2004 LCD with a PCF8574 GPIO expander backpack.
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

namespace filament_dryer {

class HD44780Display : public IDisplayDriver {
public:
    HD44780Display();
    ~HD44780Display() override;

    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "hd44780"; }
    String getName() const override { return "HD44780 LCD"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    void sleep() override;
    void wake() override;

private:
    LiquidCrystal_I2C* display_ = nullptr;
    bool initialized_    = false;
    DisplayMetrics metrics_;
    uint8_t i2c_address_ = 0x27; // Common PCF8574 address
    int8_t  sda_pin_     = 21;
    int8_t  scl_pin_     = 22;
    uint8_t cols_        = 16;
    uint8_t rows_        = 2;
    bool backlight_on_   = true;

    void renderStatus(const JsonObject& fields);
    // Print a string padded/clipped to exactly 'width' chars at (col, row)
    void printPadded(uint8_t col, uint8_t row, const String& text, uint8_t width);
};

} // namespace filament_dryer
