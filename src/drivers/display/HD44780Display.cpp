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
 * HD44780Display - Implementation
 * 16x2 or 20x4 character LCD over I2C PCF8574 backpack.
 * Character displays can only show ASCII text, no graphics.
 * Layout adapts: 16x2 shows critical fields only; 20x4 shows full dashboard.
 */
#include "HD44780Display.hpp"

namespace filament_dryer {

HD44780Display::HD44780Display() {}

HD44780Display::~HD44780Display() {
    if (display_) {
        display_->noBacklight();
        delete display_;
    }
}

bool HD44780Display::begin(const JsonObject& config) {
    if (initialized_) return true;

    i2c_address_ = config["i2c_address"] | 0x27;
    sda_pin_     = config["sda_pin"]     | 21;
    scl_pin_     = config["scl_pin"]     | 22;
    cols_        = config["cols"]        | 16;
    rows_        = config["rows"]        | 2;

    // Derive metrics from character grid
    metrics_.width       = cols_;
    metrics_.height      = rows_;
    metrics_.rotation    = 0;
    metrics_.driver_name = "HD44780";

    Wire.begin(sda_pin_, scl_pin_);
    Wire.setClock(100000); // I2C backpack max ~100 kHz for reliable operation

    display_ = new LiquidCrystal_I2C(i2c_address_, cols_, rows_);
    display_->init();

    // Verify presence via I2C ACK
    Wire.beginTransmission(i2c_address_);
    if (Wire.endTransmission() != 0) {
        delete display_;
        display_ = nullptr;
        return false;
    }

    display_->backlight();
    display_->clear();
    display_->home();

    initialized_  = true;
    backlight_on_ = true;

    return true;
}

void HD44780Display::clear() {
    if (display_) display_->clear();
}

bool HD44780Display::update(const JsonObject& status_fields) {
    if (!initialized_) return false;
    renderStatus(status_fields);
    return true;
}

void HD44780Display::showError(const String& message) {
    if (!display_) return;
    display_->clear();
    display_->setCursor(0, 0);
    display_->print("ERROR:");
    // Trim message to fit row
    String trimmed = message.substring(0, cols_);
    display_->setCursor(0, 1);
    display_->print(trimmed);
}

void HD44780Display::showBootScreen(const String& firmware_version) {
    if (!display_) return;
    display_->clear();
    display_->setCursor(0, 0);
    display_->print("Filament Dryer");
    display_->setCursor(0, 1);
    display_->print("v");
    display_->print(firmware_version);
    delay(2000);
    display_->clear();
}

DisplayMetrics HD44780Display::getMetrics() const {
    return metrics_;
}

bool HD44780Display::isConnected() {
    return initialized_ && display_ != nullptr;
}

void HD44780Display::setBrightness(uint8_t brightness) {
    // HD44780 only supports backlight on/off, not dimming
    if (display_) {
        if (brightness == 0) {
            display_->noBacklight();
            backlight_on_ = false;
        } else {
            display_->backlight();
            backlight_on_ = true;
        }
    }
}

void HD44780Display::sleep() {
    if (display_) {
        display_->noDisplay();
        display_->noBacklight();
        backlight_on_ = false;
    }
}

void HD44780Display::wake() {
    if (display_) {
        display_->display();
        display_->backlight();
        backlight_on_ = true;
    }
}

void HD44780Display::printPadded(uint8_t col, uint8_t row,
                                  const String& text, uint8_t width) {
    display_->setCursor(col, row);
    String out = text.substring(0, width);
    while ((uint8_t)out.length() < width) out += ' ';
    display_->print(out);
}

void HD44780Display::renderStatus(const JsonObject& fields) {
    if (!display_) return;

    String status = fields["status"] | "ready";

    if (rows_ == 2 && cols_ == 16) {
        // ---- 16x2 layout ----
        // Row 0: T:49.2/50C H:18%
        // Row 1: HTR:42% STATUS

        String row0 = "";
        if (fields.containsKey("chamber_temp_c")) {
            row0 += "T:" + String(fields["chamber_temp_c"].as<float>(), 1);
            row0 += "/" + String(fields["target_temp_c"] | 0.0f, 0) + "C";
        }
        if (fields.containsKey("humidity_pct")) {
            row0 += " H:" + String(fields["humidity_pct"].as<float>(), 0) + "%";
        }
        printPadded(0, 0, row0, cols_);

        String row1 = "";
        if (fields.containsKey("heater_power_pct")) {
            row1 += "H:" + String(fields["heater_power_pct"].as<float>(), 0) + "% ";
        }
        row1 += status.substring(0, cols_ - (int)row1.length());
        printPadded(0, 1, row1, cols_);

    } else if (rows_ == 4 && cols_ >= 20) {
        // ---- 20x4 layout ----
        // Row 0: Temp: 49.2 /  50.0C
        // Row 1: Hum:  18.5 /  15.0%
        // Row 2: Heater:42% Fan:80%
        // Row 3: Status: drying      

        if (fields.containsKey("chamber_temp_c")) {
            String s = "Temp: " + String(fields["chamber_temp_c"].as<float>(), 1)
                     + "/" + String(fields["target_temp_c"] | 0.0f, 0) + "C";
            printPadded(0, 0, s, cols_);
        }
        if (fields.containsKey("humidity_pct")) {
            String s = "Hum:  " + String(fields["humidity_pct"].as<float>(), 1)
                     + "/" + String(fields["target_humidity_pct"] | 0.0f, 0) + "%";
            printPadded(0, 1, s, cols_);
        }
        {
            String s = "";
            if (fields.containsKey("heater_power_pct")) {
                s += "HTR:" + String(fields["heater_power_pct"].as<float>(), 0) + "% ";
            }
            if (fields.containsKey("exhaust_fan_power_pct")) {
                s += "FAN:" + String(fields["exhaust_fan_power_pct"].as<float>(), 0) + "%";
            }
            printPadded(0, 2, s, cols_);
        }
        {
            String s = "Sts: " + status;
            printPadded(0, 3, s, cols_);
        }
    } else {
        // Generic fallback: fill rows top-down with available data
        display_->clear();
        display_->setCursor(0, 0);
        display_->print(status.substring(0, cols_));
        if (rows_ > 1 && fields.containsKey("chamber_temp_c")) {
            display_->setCursor(0, 1);
            display_->print("T:" + String(fields["chamber_temp_c"].as<float>(), 1) + "C");
        }
    }
}

} // namespace filament_dryer
