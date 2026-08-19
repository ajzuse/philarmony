/**
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
 * StateMachine - System and Drying Session State Management
 */
#pragma once

#include <Arduino.h>

namespace filament_dryer {

enum class SystemState {
    BOOT,
    WIFI_CONNECT,
    HOTSPOT,
    READY,
    DRYING,
    COOLDOWN,
    STOPPED,
    FAULT_STOPPED,
    PAUSED
};

enum class DryingStopReason {
    RUNNING,
    COMPLETED,
    USER_STOPPED,
    HUMIDITY_REACHED,
    MAX_TIME,
    SAFETY_CUTOFF,
    SENSOR_ERROR,
    THERMAL_RUNAWAY,
    PAUSE_TIMEOUT,
    POWER_LOSS
};

struct DryingSession {
    uint32_t session_id = 0;
    SystemState status = SystemState::READY;
    String profile_id;
    float target_temp_c = 0.0f;
    float target_humidity_pct = 0.0f;
    uint16_t max_duration_min = 0;
    uint32_t elapsed_sec = 0;
    uint32_t remaining_sec = 0;
    uint32_t start_timestamp = 0;
    DryingStopReason stop_reason = DryingStopReason::RUNNING;
    float current_temp_c = NAN;
    float current_humidity_pct = NAN;
    float heater_power_pct = 0.0f;
    bool heater_on = false;
    float exhaust_fan_power_pct = 0.0f;
    bool exhaust_fan_on = false;
};

class StateMachine {
public:
    static constexpr uint32_t kPauseTimeoutMs = 30UL * 60UL * 1000UL;

    StateMachine();
    ~StateMachine();

    bool begin();

    SystemState getState() const { return current_state_; }
    String getStateName() const;
    /** Lowercase status for status/update and display (boot, drying, paused, …). */
    String getStatusStreamName() const;
    static String stopReasonToString(DryingStopReason reason);

    bool transitionTo(SystemState new_state);
    bool canTransition(SystemState from, SystemState to) const;

    bool startDrying(const DryingSession& session);
    bool pauseDrying();
    bool resumeDrying();
    bool beginCooldown(DryingStopReason reason);
    bool completeCooldown();
    bool stopDrying(DryingStopReason reason);
    bool updateDryingProgress(float current_temp, float current_humidity,
                               float heater_power_pct, bool heater_on,
                               float fan_power_pct, bool fan_on);

    /** Call from control/UI loop while paused; auto-stops after 30 min. */
    bool tickPauseTimeout();

    /** Persist interrupted DRYING/PAUSED session for power-loss auto-resume. */
    bool captureInterruptedSession(DryingSession& out) const;
    bool restoreInterruptedSession(const DryingSession& session, SystemState state);

    const DryingSession& getCurrentSession() const { return current_session_; }
    DryingSession& getCurrentSession() { return current_session_; }

    bool isInFaultState() const { return current_state_ == SystemState::FAULT_STOPPED; }
    bool isDrying() const { return current_state_ == SystemState::DRYING; }
    bool isPaused() const { return current_state_ == SystemState::PAUSED; }
    bool isCoolingDown() const { return current_state_ == SystemState::COOLDOWN; }
    bool isReady() const { return current_state_ == SystemState::READY; }

    uint32_t getSessionUptime() const;
    float getProgressPercent() const;
    uint32_t getPauseElapsedMs() const;

    using StateChangeCallback = void(*)(SystemState old_state, SystemState new_state);
    using SessionUpdateCallback = void(*)(const DryingSession& session);

    void setStateChangeCallback(StateChangeCallback cb) { state_change_cb_ = cb; }
    void setSessionUpdateCallback(SessionUpdateCallback cb) { session_update_cb_ = cb; }

private:
    SystemState current_state_ = SystemState::BOOT;
    SystemState previous_state_ = SystemState::BOOT;
    DryingSession current_session_;
    DryingStopReason pending_cooldown_reason_ = DryingStopReason::COMPLETED;
    uint32_t state_enter_time_ = 0;
    uint32_t pause_enter_ms_ = 0;
    bool pause_active_ = false;
    uint32_t elapsed_at_pause_sec_ = 0;

    StateChangeCallback state_change_cb_ = nullptr;
    SessionUpdateCallback session_update_cb_ = nullptr;

    // BOOT, WIFI, HOTSPOT, READY, DRYING, COOLDOWN, STOPPED, FAULT, PAUSED
    static constexpr bool valid_transitions[9][9] = {
        // to: BOOT WIFI HOT READY DRY COOL STOP FAULT PAUSE
        {false, true,  true,  false, false, false, false, false, false}, // BOOT
        {false, false, true,  true,  false, false, false, false, false}, // WIFI
        {false, true,  false, true,  false, false, false, false, false}, // HOTSPOT
        {false, false, true,  false, true,  false, false, false, false}, // READY
        {false, false, false, false, true,  true,  true,  true,  true},  // DRYING
        {false, false, false, false, false, false, true,  true,  false}, // COOLDOWN
        {false, false, true,  true,  false, false, false, false, false}, // STOPPED
        {false, false, true,  true,  false, false, false, false, false}, // FAULT
        {false, false, false, false, true,  true,  true,  true,  false}, // PAUSED
    };
};

} // namespace filament_dryer
