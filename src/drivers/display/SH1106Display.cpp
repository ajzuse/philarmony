/**
 * SH1106Display - Implementation
 * 1.3" 128x64 OLED (I2C). The SH1106 controller has a 132-column frame buffer;
 * the Adafruit SH110X library handles the 2-pixel horizontal offset internally.
 */
#include "SH1106Display.hpp"

namespace filament_dryer {

SH1106Display::SH1106Display() {}

SH1106Display::~SH1106Display() {
    delete display_;
}

bool SH1106Display::begin(const JsonObject& config) {
    if (initialized_) return true;

    i2c_address_ = config["i2c_address"] | 0x3C;
    sda_pin_     = config["sda_pin"]     | 21;
    scl_pin_     = config["scl_pin"]     | 22;
    rst_pin_     = config["rst_pin"]     | -1;

    metrics_.width       = config["width"]    | 128;
    metrics_.height      = config["height"]   | 64;
    metrics_.rotation    = config["rotation"] | 0;
    metrics_.driver_name = "SH1106";

    Wire.begin(sda_pin_, scl_pin_);
    Wire.setClock(400000);

    if (rst_pin_ >= 0) {
        pinMode(rst_pin_, OUTPUT);
        digitalWrite(rst_pin_, LOW);
        delay(10);
        digitalWrite(rst_pin_, HIGH);
        delay(10);
    }

    display_ = new Adafruit_SH1106G(metrics_.width, metrics_.height, &Wire, rst_pin_);

    if (!display_->begin(i2c_address_, true)) {
        delete display_;
        display_ = nullptr;
        return false;
    }

    display_->clearDisplay();
    display_->setTextSize(1);
    display_->setTextColor(SH110X_WHITE);
    display_->setTextWrap(false);
    display_->display();

    initialized_ = true;
    return true;
}

void SH1106Display::clear() {
    if (display_) {
        display_->clearDisplay();
        display_->display();
    }
}

bool SH1106Display::update(const JsonObject& status_fields) {
    if (!initialized_) return false;
    display_->clearDisplay();
    renderStatus(status_fields);
    display_->display();
    return true;
}

void SH1106Display::showError(const String& message) {
    if (!display_) return;
    display_->clearDisplay();
    display_->setTextSize(1);
    display_->setTextColor(SH110X_WHITE);
    display_->setCursor(0, 0);
    display_->println("ERROR");
    display_->drawFastHLine(0, 10, display_->width(), SH110X_WHITE);
    display_->setCursor(0, 14);
    display_->println(message);
    display_->display();
}

void SH1106Display::showBootScreen(const String& firmware_version) {
    if (!display_) return;
    display_->clearDisplay();
    display_->setTextSize(2);
    display_->setTextColor(SH110X_WHITE);
    display_->setCursor(10, 10);
    display_->println("Filament");
    display_->setCursor(10, 30);
    display_->println("Dryer");
    display_->setTextSize(1);
    display_->setCursor(10, 52);
    display_->print("v");
    display_->println(firmware_version);
    display_->display();
    delay(2000);
}

DisplayMetrics SH1106Display::getMetrics() const {
    return metrics_;
}

bool SH1106Display::isConnected() {
    return initialized_ && display_ != nullptr;
}

void SH1106Display::setBrightness(uint8_t brightness) {
    current_brightness_ = brightness;
    if (display_) {
        // SH1106 contrast: 0-255 maps to 0-255 contrast register
        display_->setContrast(brightness);
    }
}

void SH1106Display::sleep() {
    if (display_) display_->oled_command(SH110X_DISPLAYOFF);
}

void SH1106Display::wake() {
    if (display_) display_->oled_command(SH110X_DISPLAYON);
}

void SH1106Display::renderStatus(const JsonObject& fields) {
    if (!display_) return;

    const int line_h = 10;
    int y = 0;

    // Title
    display_->setTextSize(1);
    display_->setTextColor(SH110X_WHITE);
    display_->setCursor(0, y); y += line_h;
    display_->print("Filament Dryer");
    display_->drawFastHLine(0, y - 2, display_->width(), SH110X_WHITE);

    auto renderField = [&](const String& label, const String& value) {
        if (y + line_h > display_->height()) return;
        display_->setCursor(0, y); y += line_h;
        display_->print(label);
        display_->print(": ");
        display_->print(value);
    };

    if (fields.containsKey("chamber_temp_c")) {
        float temp   = fields["chamber_temp_c"];
        float target = fields["target_temp_c"] | 0.0f;
        renderField("T", String(temp, 1) + "/" + String(target, 0) + "C");
    }
    if (fields.containsKey("humidity_pct")) {
        float hum    = fields["humidity_pct"];
        float target = fields["target_humidity_pct"] | 0.0f;
        renderField("H", String(hum, 1) + "/" + String(target, 0) + "%");
    }
    if (fields.containsKey("heater_power_pct")) {
        renderField("HTR", String(fields["heater_power_pct"].as<float>(), 0) + "%");
    }
    if (fields.containsKey("exhaust_fan_power_pct")) {
        renderField("FAN", String(fields["exhaust_fan_power_pct"].as<float>(), 0) + "%");
    }
    if (fields.containsKey("status")) {
        renderField("STS", fields["status"].as<String>());
    }
    if (fields.containsKey("elapsed_time_sec")) {
        uint32_t e = fields["elapsed_time_sec"];
        renderField("TME", String(e / 3600) + "h" + String((e % 3600) / 60) + "m");
    }
}

} // namespace filament_dryer
