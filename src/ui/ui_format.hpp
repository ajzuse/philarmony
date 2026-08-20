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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <Arduino.h>
#include <time.h>

#include "../core/StateMachine.hpp"

namespace filament_dryer {
namespace ui_format {

inline bool useFahrenheit(const String& temp_unit) {
    return temp_unit == "fahrenheit";
}

inline float toDisplayTemp(float celsius, bool fahrenheit) {
    return fahrenheit ? (celsius * 9.0f / 5.0f + 32.0f) : celsius;
}

inline String formatTemp(float celsius, bool fahrenheit, uint8_t decimals = 1) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.*f", static_cast<int>(decimals),
             static_cast<double>(toDisplayTemp(celsius, fahrenheit)));
    return String(buf) + (fahrenheit ? " F" : " C");
}

inline uint8_t orientationToLgfx(uint16_t degrees) {
    switch (degrees) {
        case 90:
            return 1;
        case 180:
            return 2;
        case 270:
            return 3;
        default:
            return 0;
    }
}

inline const char* resultIcon(const String& stop_reason) {
    if (stop_reason == "completed" || stop_reason == "target_humidity") {
        return "✓";
    }
    if (stop_reason == "safety_cutoff" || stop_reason == "error" ||
        stop_reason == "sensor_error") {
        return "✗";
    }
    return "⚠";
}

inline String rssiBars(int32_t rssi, bool connected) {
    if (!connected) {
        return "----";
    }
    if (rssi >= -55) return "████";
    if (rssi >= -67) return "███_";
    if (rssi >= -80) return "██__";
    return "█___";
}

inline uint32_t unixNowSec() {
    const time_t now = time(nullptr);
    if (now < 1600000000) {
        return 0;
    }
    return static_cast<uint32_t>(now);
}

inline String formatUnixDate(uint32_t unix_sec) {
    if (unix_sec < 1600000000) {
        return "--";
    }
    const time_t t = static_cast<time_t>(unix_sec);
    struct tm parts;
#if defined(ESP32)
    gmtime_r(&t, &parts);
#else
    struct tm* tmp = gmtime(&t);
    if (!tmp) return "--";
    parts = *tmp;
#endif
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
             parts.tm_year + 1900, parts.tm_mon + 1, parts.tm_mday,
             parts.tm_hour, parts.tm_min);
    return String(buf);
}

inline bool isActiveCycle(SystemState state) {
    return state == SystemState::DRYING || state == SystemState::PAUSED;
}

}  // namespace ui_format
}  // namespace filament_dryer
