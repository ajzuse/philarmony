/**
 * ST7735Display - Implementation
 * 1.44" (128x128) / 1.8" (128x160) color TFT via SPI with LovyanGFX DMA.
 */
#include "ST7735Display.hpp"

namespace filament_dryer {

ST7735Display::ST7735Display() {}

ST7735Display::~ST7735Display() {
    delete display_;
}

bool ST7735Display::begin(const JsonObject& config) {
    if (initialized_) return true;

    mosi_pin_  = config["spi_mosi"]       | 19;
    sclk_pin_  = config["spi_sclk"]       | 18;
    cs_pin_    = config["spi_cs"]         | 5;
    dc_pin_    = config["dc_pin"]         | 16;
    rst_pin_   = config["rst_pin"]        | 23;
    bl_pin_    = config["backlight_pin"]  | 4;
    width_     = config["width"]          | 128;
    height_    = config["height"]         | 160;
    rotation_  = config["rotation"]       | 1;

    metrics_.width       = width_;
    metrics_.height      = height_;
    metrics_.rotation    = rotation_;
    metrics_.driver_name = "ST7735";

    display_ = new LGFX_ST7735(mosi_pin_, sclk_pin_, cs_pin_, dc_pin_,
                                rst_pin_, bl_pin_, width_, height_, rotation_);

    if (!display_->init()) {
        Serial.println("[ST7735] Init failed");
        delete display_;
        display_ = nullptr;
        return false;
    }

    display_->setRotation(rotation_);
    display_->setBrightness(current_brightness_);
    display_->fillScreen(TFT_BLACK);
    display_->setTextColor(TFT_WHITE, TFT_BLACK);
    display_->setTextSize(1);

    initialized_ = true;
    Serial.printf("[ST7735] %dx%d initialized\n", width_, height_);
    return true;
}

void ST7735Display::clear() {
    if (display_) display_->fillScreen(TFT_BLACK);
}

bool ST7735Display::update(const JsonObject& status_fields) {
    if (!initialized_) return false;
    display_->fillScreen(TFT_BLACK);
    renderStatus(status_fields);
    return true;
}

void ST7735Display::showError(const String& message) {
    if (!display_) return;
    display_->fillScreen(TFT_RED);
    display_->setTextColor(TFT_WHITE, TFT_RED);
    display_->setTextSize(1);
    display_->setCursor(4, 4);
    display_->print("ERROR");
    display_->setCursor(4, 20);
    display_->print(message);
}

void ST7735Display::showBootScreen(const String& firmware_version) {
    if (!display_) return;
    display_->fillScreen(TFT_BLACK);
    display_->setTextColor(TFT_CYAN, TFT_BLACK);
    display_->setTextSize(2);
    display_->setCursor(4, 30);
    display_->print("Filament");
    display_->setCursor(4, 56);
    display_->print("Dryer");
    display_->setTextSize(1);
    display_->setTextColor(TFT_WHITE, TFT_BLACK);
    display_->setCursor(4, 90);
    display_->print("v");
    display_->print(firmware_version);
    delay(2000);
}

DisplayMetrics ST7735Display::getMetrics() const {
    return metrics_;
}

bool ST7735Display::isConnected() {
    return initialized_ && display_ != nullptr;
}

void ST7735Display::setBrightness(uint8_t brightness) {
    current_brightness_ = brightness;
    if (display_) display_->setBrightness(brightness);
}

void ST7735Display::sleep() {
    if (display_) display_->sleep();
}

void ST7735Display::wake() {
    if (display_) display_->wakeup();
}

void ST7735Display::renderStatus(const JsonObject& fields) {
    if (!display_) return;

    // Header bar
    display_->fillRect(0, 0, width_, 16, TFT_NAVY);
    display_->setTextColor(TFT_CYAN, TFT_NAVY);
    display_->setTextSize(1);
    display_->setCursor(2, 4);
    display_->print("Filament Dryer");

    int y = 20;
    const int line_h = 18;

    auto drawField = [&](const String& label, const String& value, uint16_t col = TFT_WHITE) {
        if (y + line_h > height_) return;
        display_->setTextColor(TFT_DARKGREY, TFT_BLACK);
        display_->setCursor(2, y);
        display_->print(label);
        display_->setTextColor(col, TFT_BLACK);
        display_->setCursor(50, y);
        display_->print(value);
        y += line_h;
    };

    if (fields.containsKey("chamber_temp_c")) {
        float t = fields["chamber_temp_c"], tgt = fields["target_temp_c"] | 0.0f;
        drawField("Temp:", String(t, 1) + "/" + String(tgt, 0) + "C",
                  t > tgt ? TFT_RED : TFT_GREEN);
    }
    if (fields.containsKey("humidity_pct")) {
        float h = fields["humidity_pct"], tgt = fields["target_humidity_pct"] | 0.0f;
        drawField("Hum:", String(h, 1) + "/" + String(tgt, 0) + "%", TFT_CYAN);
    }
    if (fields.containsKey("heater_power_pct")) {
        float p = fields["heater_power_pct"];
        drawField("Heater:", String(p, 0) + "%", p > 50 ? TFT_ORANGE : TFT_GREEN);
    }
    if (fields.containsKey("exhaust_fan_power_pct")) {
        drawField("Fan:", String(fields["exhaust_fan_power_pct"].as<float>(), 0) + "%", TFT_BLUE);
    }
    if (fields.containsKey("status")) {
        String s = fields["status"].as<String>();
        uint16_t col = (s == "drying") ? TFT_GREEN : (s == "fault_stopped") ? TFT_RED : TFT_YELLOW;
        drawField("Status:", s, col);
    }
    if (fields.containsKey("elapsed_time_sec")) {
        uint32_t e = fields["elapsed_time_sec"];
        drawField("Time:", String(e / 3600) + "h " + String((e % 3600) / 60) + "m");
    }
}

} // namespace filament_dryer
