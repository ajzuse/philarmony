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

#include "Xpt2046Touch.hpp"

namespace filament_dryer {

bool Xpt2046Touch::begin(const JsonObject& config) {
    cs_pin_ = config["spi_cs"] | -1;
    irq_pin_ = config["irq_pin"] | -1;
    initialized_ = cs_pin_ >= 0;
    if (!initialized_) {
        last_error_ = "XPT2046 requires spi_cs";
        return false;
    }

    pinMode(cs_pin_, OUTPUT);
    digitalWrite(cs_pin_, HIGH);
    if (irq_pin_ >= 0) {
        pinMode(irq_pin_, INPUT_PULLUP);
    }
    last_error_ = "";
    return true;
}

TouchPoint Xpt2046Touch::read() {
    TouchPoint point;
    point.timestamp_ms = millis();
    // SPI sampling will be supplied with the board-specific display bus port.
    point.pressed = false;
    return point;
}

bool Xpt2046Touch::isConnected() {
    return initialized_;
}

void Xpt2046Touch::setSensitivity(const String& level) {
    sensitivity_ = (level == "low" || level == "high") ? level : String("medium");
}

} // namespace filament_dryer
