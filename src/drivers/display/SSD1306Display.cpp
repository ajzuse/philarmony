/**
 * SSD1306Display - Implementation
 */
#include "SSD1306Display.hpp"

namespace filament_dryer {

SSD1306Display::SSD1306Display() {}

SSD1306Display::~SSD1306Display() {
    delete display_;
}

bool SSD1306Display::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    i2c_address_ = config["i2c_address"] | 0x3C;
    sda_pin_ = config["sda_pin"] | 21;
    scl_pin_ = config["scl_pin"] | 22;
    rst_pin_ = config["rst_pin"] | -1;
    
    metrics_.width = config["width"] | 128;
    metrics_.height = config["height"] | 64;
    metrics_.rotation = config["rotation"] | 0;
    metrics_.driver_name = "SSD1306";
    
    Wire.begin(sda_pin_, scl_pin_);
    Wire.setClock(400000);
    
    // Reset sequence
    if (rst_pin_ >= 0) {
        pinMode(rst_pin_, OUTPUT);
        digitalWrite(rst_pin_, LOW);
        delay(10);
        digitalWrite(rst_pin_, HIGH);
        delay(10);
    }
    
    display_ = new Adafruit_SSD1306(metrics_.width, metrics_.height, &Wire, rst_pin_);
    
    if (!display_->begin(SSD1306_SWITCHCAPVCC, i2c_address_)) {
        return false;
    }
    
    display_->clearDisplay();
    display_->setTextSize(1);
    display_->setTextColor(SSD1306_WHITE);
    display_->setTextWrap(false);
    display_->cp437(true);
    display_->display();
    
    initialized_ = true;
    return true;
}

void SSD1306Display::clear() {
    if (display_) {
        display_->clearDisplay();
        display_->display();
    }
}

bool SSD1306Display::update(const JsonObject& status_fields) {
    if (!initialized_) return false;
    
    display_->clearDisplay();
    renderStatus(status_fields);
    display_->display();
    return true;
}

void SSD1306Display::showError(const String& message) {
    if (!display_) return;
    
    display_->clearDisplay();
    display_->setTextSize(1);
    display_->setTextColor(SSD1306_WHITE);
    display_->setCursor(0, 0);
    display_->println("ERROR");
    display_->drawFastHLine(0, 10, display_->width(), SSD1306_WHITE);
    display_->setCursor(0, 14);
    display_->println(message);
    display_->display();
}

void SSD1306Display::showBootScreen(const String& firmware_version) {
    if (!display_) return;
    
    display_->clearDisplay();
    display_->setTextSize(2);
    display_->setTextColor(SSD1306_WHITE);
    display_->setCursor(10, 10);
    display_->println("Filament");
    display_->setCursor(10, 30);
    display_->println("Dryer");
    display_->setTextSize(1);
    display_->setCursor(10, 50);
    display_->print("v");
    display_->println(firmware_version);
    display_->display();
    delay(2000);
}

DisplayMetrics SSD1306Display::getMetrics() const {
    return metrics_;
}

bool SSD1306Display::isConnected() {
    return initialized_ && display_ != nullptr;
}

void SSD1306Display::setBrightness(uint8_t brightness) {
    if (display_) {
        display_->dim(brightness < 128);
        current_brightness_ = brightness;
    }
}

void SSD1306Display::sleep() {
    if (display_) {
        display_->ssd1306_command(SSD1306_DISPLAYOFF);
    }
}

void SSD1306Display::wake() {
    if (display_) {
        display_->ssd1306_command(SSD1306_DISPLAYON);
    }
}

void SSD1306Display::renderStatus(const JsonObject& fields) {
    if (!display_) return;
    
    int y = 0;
    const int line_height = 10;
    int row = 0;
    
    // Title bar
    display_->setTextSize(1);
    display_->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display_->setCursor(0, 0);
    display_->print("Filament Dryer");
    display_->setCursor(display_->width() - 30, 0);
    display_->print("v0.1");
    
    // Divider
    display_->drawFastHLine(0, 12, display_->width(), SSD1306_WHITE);
    
    // Status fields
    int x = 0;
    y = 16;
    
    // Helper lambda for field rendering
    auto renderField = [&](const String& label, const String& value) {
        if (y + line_height > display_->height()) return;
        display_->setCursor(x, y);
        display_->print(label);
        display_->print(": ");
        display_->print(value);
        y += line_height;
    };
    
    // Temperature
    if (fields.containsKey("chamber_temp_c")) {
        float temp = fields["chamber_temp_c"];
        float target = fields["target_temp_c"] | 0;
        renderField("T", String(temp, 1) + "/" + String(target, 0) + "C");
    }
    
    // Humidity
    if (fields.containsKey("humidity_pct")) {
        float hum = fields["humidity_pct"];
        float target = fields["target_humidity_pct"] | 0;
        renderField("H", String(hum, 1) + "/" + String(target, 0) + "%");
    }
    
    // Heater
    if (fields.containsKey("heater_power_pct")) {
        float pwm = fields["heater_power_pct"];
        renderField("HTR", String(pwm, 0) + "%");
    }
    
    // Fan
    if (fields.containsKey("exhaust_fan_power_pct")) {
        float pwm = fields["exhaust_fan_power_pct"];
        renderField("FAN", String(pwm, 0) + "%");
    }
    
    // Status
    if (fields.containsKey("status")) {
        renderField("STS", fields["status"].as<String>());
    }
    
    // Elapsed time
    if (fields.containsKey("elapsed_time_sec")) {
        uint32_t elapsed = fields["elapsed_time_sec"];
        uint32_t h = elapsed / 3600;
        uint32_t m = (elapsed % 3600) / 60;
        renderField("TIME", String(h) + "h" + String(m) + "m");
    }
}

} // namespace filament_dryer