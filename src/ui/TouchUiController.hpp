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
 * TouchUiController - Validated command bridge from touch UI to firmware core
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "../core/ConfigManager.hpp"
#include "../core/ProfileManager.hpp"
#include "../core/StateMachine.hpp"

namespace filament_dryer {

class TouchUiController {
public:
    using BroadcastCallback = void (*)(const JsonObject& status);

    TouchUiController(StateMachine& state_machine, ProfileManager& profile_manager,
                      ConfigManager& config_manager);

    bool startFromProfile(const String& profile_id);
    bool startCustom(float temperature_c, float humidity_pct,
                     uint16_t duration_min);
    bool pause();
    bool resume();
    bool stop();
    bool applyTargets(float temperature_c, float humidity_pct);

    void tick();
    String getUiSource() const { return "touch"; }
    void setBroadcastCallback(BroadcastCallback callback) {
        broadcast_callback_ = callback;
    }

private:
    static constexpr uint32_t kPauseTimeoutMs = 30UL * 60UL * 1000UL;

    StateMachine& state_machine_;
    ProfileManager& profile_manager_;
    ConfigManager& config_manager_;
    BroadcastCallback broadcast_callback_ = nullptr;
    uint32_t pause_started_ms_ = 0;
    bool pause_timer_active_ = false;

    void broadcastStatus();
};

} // namespace filament_dryer
