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
 * AmbientFanLogic - Pure, hardware-independent ambient ventilation decisions
 *
 * Extracted from the control loop so the decision can be exercised by host
 * flow tests without pulling in Arduino/FreeRTOS dependencies.
 */
#pragma once

#include <cmath>
#include <cstdint>

namespace filament_dryer {
namespace ambient_fan {

inline constexpr uint32_t kAmbientTimeoutMs = 30000;
inline constexpr float kMarginMinPct = 0.0f;
inline constexpr float kMarginMaxPct = 10.0f;
inline constexpr float kMarginDefaultPct = 2.0f;

/**
 * True when a valid ambient reading has been seen within the timeout window.
 * @param have_valid_reading whether any valid ambient reading was ever seen
 * @param now_ms current millis() timestamp
 * @param last_valid_ms timestamp of the last valid ambient reading
 * @param timeout_ms maximum age before ambient is treated as unavailable
 */
inline bool ambientIsAvailable(bool have_valid_reading, uint32_t now_ms,
                               uint32_t last_valid_ms,
                               uint32_t timeout_ms = kAmbientTimeoutMs) {
    if (!have_valid_reading) {
        return false;
    }
    return (now_ms - last_valid_ms) < timeout_ms;
}

/**
 * True when humidity-reduction ventilation is useless because chamber humidity
 * is within the margin of (or below) ambient humidity.
 */
inline bool shouldStopFanForAmbient(bool ambient_available,
                                    float chamber_humidity,
                                    float ambient_humidity,
                                    float margin_pct) {
    if (!ambient_available) {
        return false;
    }
    if (std::isnan(chamber_humidity) || std::isnan(ambient_humidity)) {
        return false;
    }
    return chamber_humidity <= ambient_humidity + margin_pct;
}

/**
 * Clamp the configurable useless-ventilation margin to the supported 0-10 %RH
 * range, falling back to the default when the stored value is not a number.
 */
inline float clampAmbientMarginPct(float margin_pct) {
    if (std::isnan(margin_pct)) {
        return kMarginDefaultPct;
    }
    if (margin_pct < kMarginMinPct) {
        return kMarginMinPct;
    }
    if (margin_pct > kMarginMaxPct) {
        return kMarginMaxPct;
    }
    return margin_pct;
}

}  // namespace ambient_fan
}  // namespace filament_dryer
