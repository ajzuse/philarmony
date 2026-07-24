/**
 * DisplayManager - Implementation
 * Auto-detection and multi-display abstraction layer
 */
#include "DisplayManager.hpp"

namespace filament_dryer {

DisplayManager::DisplayManager() {}

DisplayManager::~DisplayManager() {}

bool DisplayManager::begin(const JsonObject& config) {
    // Try to auto-detect display based on hardware
    // This will be called with proper config in setup()
    return active_display_ != nullptr;
}

bool DisplayManager::autoDetect(const JsonObject& config) {
    // Try displays in order of likelihood for common ESP32 boards
    // 1. ST7789 (LilyGo T-Display, T-QT)
    if (tryDetectDisplay(DetectOrder::ST7789, config)) return true;
    
    // 2. ILI9341 (CYD board)
    if (tryDetectDisplay(DetectOrder::ILI9341, config)) return true;
    
    // 3. SSD1306 (generic I2C OLED)
    if (tryDetectDisplay(DetectOrder::SSD1306, config)) return true;
    
    return false;
}

bool DisplayManager::tryDetectDisplay(DetectOrder order, const JsonObject& config) {
    JsonObject dispConfig = config;
    dispConfig["enabled"] = true;
    
    switch (order) {
        case DetectOrder::ST7789: {
            // ST7789 typically on SPI with specific pins
            dispConfig["driver"] = "st7789";
            dispConfig["bus_type"] = "spi";
            if (st7789_.begin(dispConfig)) {
                if (st7789_.isConnected()) {
                    active_display_ = &st7789_;
                    active_type_ = "st7789";
                    logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, "Auto-detected ST7789 display");
                    return true;
                }
            }
            break;
        }
        case DetectOrder::ILI9341: {
            dispConfig["driver"] = "ili9341";
            dispConfig["bus_type"] = "spi";
            if (ili9341_.begin(dispConfig)) {
                if (ili9341_.isConnected()) {
                    active_display_ = &ili9341_;
                    active_type_ = "ili9341";
                    logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, "Auto-detected ILI9341 display");
                    return true;
                }
            }
            break;
        }
        case DetectOrder::SSD1306: {
            dispConfig["driver"] = "ssd1306";
            dispConfig["bus_type"] = "i2c";
            if (ssd1306_.begin(dispConfig)) {
                if (ssd1306_.isConnected()) {
                    active_display_ = &ssd1306_;
                    active_type_ = "ssd1306";
                    logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, "Auto-detected SSD1306 display");
                    return true;
                }
            }
            break;
        }
        default:
            break;
    }
    
    return false;
}

bool DisplayManager::setDisplayType(const String& type, const JsonObject& config) {
    JsonObject dispConfig = config;
    dispConfig["enabled"] = true;
    dispConfig["driver"] = type;
    
    if (type == "st7789") {
        if (st7789_.begin(dispConfig) && st7789_.isConnected()) {
            active_display_ = &st7789_;
            active_type_ = "st7789";
            return true;
        }
    } else if (type == "ili9341") {
        if (ili9341_.begin(dispConfig) && ili9341_.isConnected()) {
            active_display_ = &ili9341_;
            active_type_ = "ili9341";
            return true;
        }
    } else if (type == "ssd1306" || type == "sh1106") {
        if (ssd1306_.begin(dispConfig) && ssd1306_.isConnected()) {
            active_display_ = &ssd1306_;
            active_type_ = "ssd1306";
            return true;
        }
    }
    
    return false;
}

void DisplayManager::update(const JsonObject& status_fields) {
    if (active_display_ && active_display_->isConnected()) {
        active_display_->update(status_fields);
    }
}

void DisplayManager::clear() {
    if (active_display_) {
        active_display_->clear();
    }
}

void DisplayManager::showError(const String& message) {
    if (active_display_) {
        active_display_->showError(message);
    }
}

void DisplayManager::showBootScreen(const String& firmware_version) {
    if (active_display_) {
        active_display_->showBootScreen(firmware_version);
    }
}

bool DisplayManager::isAnyConnected() const {
    return active_display_ && active_display_->isConnected();
}

void DisplayManager::setLayout(const JsonObject& layout_config) {
    if (layout_config.containsKey("fields")) {
        JsonArray arr = layout_config["fields"];
        layout_.fields.clear();
        for (JsonVariant v : arr) {
            layout_.fields.push_back(v.as<String>());
        }
    }
    layout_.show_graph = layout_config["show_graph"] | false;
    layout_.font_size = layout_config["font_size"] | 1;
    layout_.compact_mode = layout_config["compact_mode"] | false;
}

String DisplayManager::getLayoutPreview() const {
    String preview = "Active: " + active_type_ + "\n";
    preview += "Fields: ";
    for (size_t i = 0; i < layout_.fields.size(); i++) {
        if (i > 0) preview += ", ";
        preview += layout_.fields[i];
    }
    return preview;
}

} // namespace filament_dryer