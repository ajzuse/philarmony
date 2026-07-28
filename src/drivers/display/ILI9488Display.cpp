/**
 * ILI9488Display - Implementation
 * 3.5" / 4.0" 320x480 TFT with LovyanGFX SPI DMA.
 * Large screen allows full data dashboard layout.
 */
#include "ILI9488Display.hpp"

namespace filament_dryer {

ILI9488Display::ILI9488Display() {}

ILI9488Display::~ILI9488Display() {
    delete display_;
}

bool ILI9488Display::begin(const JsonObject& config) {
    if (initialized_) return true;

    mosi_pin_  = config["spi_mosi"]      | 19;
    sclk_pin_  = config["spi_sclk"]      | 18;
    cs_pin_    = config["spi_cs"]        | 5;
    dc_pin_    = config["dc_pin"]        | 16;
    rst_pin_   = config["rst_pin"]       | 23;
    bl_pin_    = config["backlight_pin"] | 4;
    width_     = config["width"]         | 320;
    height_    = config["height"]        | 480;
    rotation_  = config["rotation"]      | 1;

    metrics_.width       = width_;
    metrics_.height      = height_;
    metrics_.rotation    = rotation_;
    metrics_.driver_name = "ILI9488";

    display_ = new LGFX_ILI9488(mosi_pin_, sclk_pin_, cs_pin_, dc_pin_,
                                  rst_pin_, bl_pin_, width_, height_, rotation_);

    if (!display_->init()) {
        Serial.println("[ILI9488] Init failed");
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
    Serial.printf("[ILI9488] %dx%d initialized\n", width_, height_);
    return true;
}

void ILI9488Display::clear() {
    if (display_) display_->fillScreen(TFT_BLACK);
}

bool ILI9488Display::update(const JsonObject& status_fields) {
    if (!initialized_) return false;
    display_->fillScreen(TFT_BLACK);
    renderStatus(status_fields);
    return true;
}

void ILI9488Display::showError(const String& message) {
    if (!display_) return;
    display_->fillScreen(TFT_RED);
    display_->setTextColor(TFT_WHITE, TFT_RED);
    display_->setTextSize(3);
    display_->setCursor(10, 80);
    display_->print("ERROR");
    display_->setTextSize(2);
    display_->setCursor(10, 130);
    display_->print(message);
}

void ILI9488Display::showBootScreen(const String& firmware_version) {
    if (!display_) return;
    display_->fillScreen(TFT_BLACK);
    display_->setTextColor(TFT_CYAN, TFT_BLACK);
    display_->setTextSize(4);
    display_->setCursor(20, 100);
    display_->print("Filament");
    display_->setCursor(20, 150);
    display_->print("Dryer");
    display_->setTextSize(2);
    display_->setTextColor(TFT_WHITE, TFT_BLACK);
    display_->setCursor(20, 220);
    display_->print("v");
    display_->print(firmware_version);
    delay(2000);
}

DisplayMetrics ILI9488Display::getMetrics() const {
    return metrics_;
}

bool ILI9488Display::isConnected() {
    return initialized_ && display_ != nullptr;
}

void ILI9488Display::setBrightness(uint8_t brightness) {
    current_brightness_ = brightness;
    if (display_) display_->setBrightness(brightness);
}

void ILI9488Display::sleep() {
    if (display_) display_->sleep();
}

void ILI9488Display::wake() {
    if (display_) display_->wakeup();
}

void ILI9488Display::drawField(int x, int y, const String& label,
                                const String& value, uint16_t color) {
    display_->setTextSize(2);
    display_->setTextColor(TFT_DARKGREY, TFT_BLACK);
    display_->setCursor(x, y);
    display_->print(label);
    display_->setTextColor(color, TFT_BLACK);
    display_->setCursor(x + 120, y);
    display_->print(value);
}

void ILI9488Display::renderStatus(const JsonObject& fields) {
    if (!display_) return;

    // Header bar
    display_->fillRect(0, 0, width_, 40, TFT_NAVY);
    display_->setTextColor(TFT_CYAN, TFT_NAVY);
    display_->setTextSize(2);
    display_->setCursor(10, 10);
    display_->print("Filament Dryer ESP32");

    // Horizontal separator
    display_->drawFastHLine(0, 42, width_, TFT_CYAN);

    int y = 60;
    const int row_h = 44;

    if (fields.containsKey("chamber_temp_c")) {
        float t = fields["chamber_temp_c"], tgt = fields["target_temp_c"] | 0.0f;
        drawField(10, y, "Temp:", String(t, 1) + " / " + String(tgt, 0) + " C",
                  t > tgt ? TFT_RED : TFT_GREEN);
        y += row_h;
    }
    if (fields.containsKey("humidity_pct")) {
        float h = fields["humidity_pct"], tgt = fields["target_humidity_pct"] | 0.0f;
        drawField(10, y, "Hum:", String(h, 1) + " / " + String(tgt, 0) + " %", TFT_CYAN);
        y += row_h;
    }
    if (fields.containsKey("heater_power_pct")) {
        float p = fields["heater_power_pct"];
        drawField(10, y, "Heater:", String(p, 0) + " %",
                  p > 70 ? TFT_ORANGE : TFT_GREEN);
        y += row_h;
    }
    if (fields.containsKey("exhaust_fan_power_pct")) {
        drawField(10, y, "Fan:",
                  String(fields["exhaust_fan_power_pct"].as<float>(), 0) + " %", TFT_BLUE);
        y += row_h;
    }
    if (fields.containsKey("status")) {
        String s = fields["status"].as<String>();
        uint16_t col = (s == "drying") ? TFT_GREEN : (s == "fault_stopped") ? TFT_RED : TFT_YELLOW;
        drawField(10, y, "Status:", s, col);
        y += row_h;
    }
    if (fields.containsKey("elapsed_time_sec")) {
        uint32_t e = fields["elapsed_time_sec"];
        uint32_t r = fields["remaining_time_sec"] | 0u;
        drawField(10, y, "Elapsed:",
                  String(e / 3600) + "h " + String((e % 3600) / 60) + "m", TFT_WHITE);
        y += row_h;
        drawField(10, y, "Remain:",
                  String(r / 3600) + "h " + String((r % 3600) / 60) + "m", TFT_WHITE);
        y += row_h;
    }
    if (fields.containsKey("free_heap_bytes")) {
        uint32_t heap = fields["free_heap_bytes"];
        drawField(10, y, "Free RAM:", String(heap / 1024) + " KB", TFT_DARKGREY);
    }
}

} // namespace filament_dryer
