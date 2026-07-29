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
 * NextionDisplay - Implementation
 * Sends data updates over UART to Nextion HMI display.
 * The Nextion protocol requires every command terminated by three 0xFF bytes.
 */
#include "NextionDisplay.hpp"

namespace filament_dryer {

NextionDisplay::NextionDisplay() {}

NextionDisplay::~NextionDisplay() {
    if (serial_) {
        serial_->end();
        // HardwareSerial instances are statically allocated; do not delete
    }
}

bool NextionDisplay::begin(const JsonObject& config) {
    if (initialized_) return true;

    rx_pin_      = config["rx_pin"]    | 16;
    tx_pin_      = config["tx_pin"]    | 17;
    baudrate_    = config["baudrate"]  | 9600;
    page_prefix_ = config["page"]      | "page0";

    metrics_.width       = config["width"]    | 320;
    metrics_.height      = config["height"]   | 240;
    metrics_.rotation    = config["rotation"] | 0;
    metrics_.driver_name = "Nextion";

    // Use UART2 (Serial2) for the Nextion; avoids conflicts with debug Serial
    serial_ = &Serial2;
    serial_->begin(baudrate_, SERIAL_8N1, rx_pin_, tx_pin_);

    // Send reset + baud-rate negotiation
    sendCmd(""); // Wake up / flush
    sendCmd("baud=" + String(baudrate_));

    // Set brightness (0-100 on Nextion)
    sendCmd("dim=" + String(current_brightness_));

    // Page 0 is expected to be the main status page
    sendCmd("page " + page_prefix_);

    initialized_ = true;
    return true;
}

void NextionDisplay::clear() {
    // Nextion manages its own rendering; clearing means blanking text components
    if (!initialized_) return;
    setAttr(page_prefix_ + ".t_temp",   "txt", "");
    setAttr(page_prefix_ + ".t_tgt",    "txt", "");
    setAttr(page_prefix_ + ".t_hum",    "txt", "");
    setAttr(page_prefix_ + ".t_htr",    "txt", "");
    setAttr(page_prefix_ + ".t_fan",    "txt", "");
    setAttr(page_prefix_ + ".t_status", "txt", "");
    setAttr(page_prefix_ + ".t_time",   "txt", "");
}

bool NextionDisplay::update(const JsonObject& status_fields) {
    if (!initialized_) return false;

    if (status_fields.containsKey("chamber_temp_c")) {
        float t   = status_fields["chamber_temp_c"];
        float tgt = status_fields["target_temp_c"] | 0.0f;
        setAttr(page_prefix_ + ".t_temp", "txt",
                String(t, 1) + "C");
        setAttr(page_prefix_ + ".t_tgt",  "txt",
                ">" + String(tgt, 0) + "C");
        // Numeric component for progress bar (0-100 = 0-80°C scale)
        setVal(page_prefix_ + ".n_temp", static_cast<int32_t>(t));
        setVal(page_prefix_ + ".n_tgt",  static_cast<int32_t>(tgt));
    }

    if (status_fields.containsKey("humidity_pct")) {
        float h   = status_fields["humidity_pct"];
        float tgt = status_fields["target_humidity_pct"] | 0.0f;
        setAttr(page_prefix_ + ".t_hum", "txt",
                String(h, 1) + "% >" + String(tgt, 0) + "%");
        setVal(page_prefix_ + ".n_hum", static_cast<int32_t>(h));
    }

    if (status_fields.containsKey("heater_power_pct")) {
        float pwr = status_fields["heater_power_pct"];
        setAttr(page_prefix_ + ".t_htr", "txt",
                "HTR " + String(pwr, 0) + "%");
        setVal(page_prefix_ + ".j_htr", static_cast<int32_t>(pwr)); // progress bar
    }

    if (status_fields.containsKey("exhaust_fan_power_pct")) {
        float fan = status_fields["exhaust_fan_power_pct"];
        setAttr(page_prefix_ + ".t_fan", "txt",
                "FAN " + String(fan, 0) + "%");
        setVal(page_prefix_ + ".j_fan", static_cast<int32_t>(fan));
    }

    if (status_fields.containsKey("status")) {
        setAttr(page_prefix_ + ".t_status", "txt",
                status_fields["status"].as<String>());
    }

    if (status_fields.containsKey("elapsed_time_sec")) {
        uint32_t e = status_fields["elapsed_time_sec"];
        String ts  = String(e / 3600) + "h " + String((e % 3600) / 60) + "m";
        setAttr(page_prefix_ + ".t_time", "txt", ts);
    }

    return true;
}

void NextionDisplay::showError(const String& message) {
    if (!initialized_) return;
    // Navigate to the error page if it exists; fall back to updating status text
    sendCmd("page error");
    setAttr("error.t_msg", "txt", message.substring(0, 64));
}

void NextionDisplay::showBootScreen(const String& firmware_version) {
    if (!initialized_) return;
    sendCmd("page boot");
    setAttr("boot.t_ver", "txt", "v" + firmware_version);
    sendCmd("page " + page_prefix_);
}

DisplayMetrics NextionDisplay::getMetrics() const {
    return metrics_;
}

bool NextionDisplay::isConnected() {
    // Best-effort: check that serial was opened
    return initialized_ && serial_ != nullptr;
}

void NextionDisplay::setBrightness(uint8_t brightness) {
    // Nextion dim command accepts 0-100
    current_brightness_ = (brightness * 100) / 255;
    if (initialized_) {
        sendCmd("dim=" + String(current_brightness_));
    }
}

void NextionDisplay::sleep() {
    if (initialized_) sendCmd("sleep=1");
}

void NextionDisplay::wake() {
    if (initialized_) sendCmd("sleep=0");
}

// ---- Private helpers ----

void NextionDisplay::sendCmd(const String& cmd) {
    if (!serial_) return;
    serial_->print(cmd);
    serial_->write(0xFF);
    serial_->write(0xFF);
    serial_->write(0xFF);
}

void NextionDisplay::setAttr(const String& component, const String& attr,
                              const String& value) {
    // Nextion syntax: component.attr="value"
    sendCmd(component + "." + attr + "=\"" + value + "\"");
}

void NextionDisplay::setVal(const String& component, int32_t value) {
    // Nextion syntax: component.val=number
    sendCmd(component + ".val=" + String(value));
}

} // namespace filament_dryer
