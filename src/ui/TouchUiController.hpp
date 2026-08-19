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
#include "../core/SafetyEngine.hpp"
#include "../core/StateMachine.hpp"

namespace filament_dryer {

class TouchUiController {
public:
    using BroadcastCallback = void (*)(const JsonObject& status);
    using WifiHotspotCallback = void (*)();

    TouchUiController(StateMachine& state_machine, ProfileManager& profile_manager,
                      ConfigManager& config_manager,
                      SafetyEngine* safety = nullptr);

    bool startFromProfile(const String& profile_id);
    bool startCustom(float temperature_c, float humidity_pct,
                     uint16_t duration_min);
    bool pause();
    bool resume();
    bool stop();
    bool applyTargets(float temperature_c, float humidity_pct,
                      uint16_t duration_min);

    std::vector<FilamentProfile> listProfiles() const;
    ConfigManager& config() { return config_manager_; }
    const ConfigManager& config() const { return config_manager_; }
    UISettings getUiSettings() const;
    TouchConfig getTouchConfig() const;
    bool applyUiSettings(const UISettings& settings);
    bool applyTouchConfig(const TouchConfig& config);
    void requestWifiHotspot();
    void setWifiHotspotCallback(WifiHotspotCallback callback) {
        wifi_hotspot_callback_ = callback;
    }

    SystemState getState() const { return state_machine_.getState(); }
    const DryingSession& getCurrentSession() const {
        return state_machine_.getCurrentSession();
    }
    bool consumePauseTimeoutEvent();

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
    SafetyEngine* safety_ = nullptr;
    BroadcastCallback broadcast_callback_ = nullptr;
    WifiHotspotCallback wifi_hotspot_callback_ = nullptr;
    uint32_t pause_started_ms_ = 0;
    bool pause_timer_active_ = false;
    bool pause_timeout_pending_ = false;
    bool pause_timeout_reported_ = false;

    void broadcastStatus();
    void broadcastAck(const char* topic, bool success);
    bool safetyAllows(float temperature_c) const;
};

} // namespace filament_dryer
