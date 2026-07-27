/**
 * DisplayManager - Auto-detect and manage multiple display drivers
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include "drivers/display/SSD1306Display.hpp"
#include "drivers/display/ST7789Display.hpp"
#include "drivers/display/ILI9341Display.hpp"
#include "LogManager.hpp"

namespace filament_dryer {

class DisplayManager {
public:
    enum class DetectOrder {
        ST7789,
        ILI9341,
        SSD1306
    };
    
    struct LayoutConfig {
        std::vector<String> fields;
        bool show_graph = false;
        uint8_t font_size = 1;
        bool compact_mode = false;
    };
    
    DisplayManager();
    ~DisplayManager();
    
    bool begin(const JsonObject& config);
    bool setDisplayType(const String& type, const JsonObject& config);
    
    void update(const JsonObject& status_fields);
    void clear();
    void showError(const String& message);
    void showBootScreen(const String& firmware_version);
    bool isAnyConnected() const;
    
    void setLayout(const JsonObject& layout_config);
    String getLayoutPreview() const;
    
    String getActiveType() const { return active_type_; }

private:
    bool tryDetectDisplay(DetectOrder order, const JsonObject& config);
    
    // Display instances
    SSD1306Display ssd1306_;
    ST7789Display st7789_;
    ILI9341Display ili9341_;
    
    // Active display
    IDisplayDriver* active_display_ = nullptr;
    String active_type_ = "none";
    LayoutConfig layout_;
};

} // namespace filament_dryer