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
 * Nextion UART HMI Display Driver
 * Communicates with Nextion displays over UART using the Nextion instruction set.
 * The Nextion HMI renders its own GUI; this driver sends data updates to
 * named component variables on the active Nextion page.
 *
 * Expected Nextion page/component naming convention:
 *   page0.t_temp.txt   - chamber temperature string
 *   page0.t_tgt.txt    - target temperature string
 *   page0.t_hum.txt    - humidity string
 *   page0.t_htr.txt    - heater power string
 *   page0.t_fan.txt    - fan power string
 *   page0.t_status.txt - status string
 *   page0.t_time.txt   - elapsed time string
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <HardwareSerial.h>

namespace filament_dryer {

class NextionDisplay : public IDisplayDriver {
public:
    NextionDisplay();
    ~NextionDisplay() override;

    bool begin(const JsonObject& config) override;
    void clear() override;
    bool update(const JsonObject& status_fields) override;
    void showError(const String& message) override;
    void showBootScreen(const String& firmware_version) override;
    DisplayMetrics getMetrics() const override;
    String getType() const override { return "nextion"; }
    String getName() const override { return "Nextion HMI"; }
    bool isConnected() override;
    void setBrightness(uint8_t brightness) override;
    void sleep() override;
    void wake() override;

private:
    HardwareSerial* serial_ = nullptr;
    bool initialized_       = false;
    DisplayMetrics metrics_;
    int8_t  rx_pin_         = 16;
    int8_t  tx_pin_         = 17;
    uint32_t baudrate_      = 9600;
    String  page_prefix_    = "page0";
    uint8_t current_brightness_ = 100; // Nextion brightness 0-100

    // Send a Nextion command followed by the required 3x 0xFF terminator
    void sendCmd(const String& cmd);
    // Set a Nextion text component: setAttr("page0.t_temp", "txt", "49.2C")
    void setAttr(const String& component, const String& attr, const String& value);
    // Set a Nextion numeric component value
    void setVal(const String& component, int32_t value);
};

} // namespace filament_dryer
