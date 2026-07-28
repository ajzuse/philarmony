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
 * StateMachine - Implementation
 */
#include "StateMachine.hpp"

namespace filament_dryer {

constexpr bool StateMachine::valid_transitions[8][8];

StateMachine::StateMachine() {}

StateMachine::~StateMachine() {}

bool StateMachine::begin() {
    current_state_ = SystemState::BOOT;
    state_enter_time_ = millis();
    return true;
}

String StateMachine::getStateName() const {
    switch (current_state_) {
        case SystemState::BOOT: return "BOOT";
        case SystemState::WIFI_CONNECT: return "WIFI_CONNECT";
        case SystemState::HOTSPOT: return "HOTSPOT";
        case SystemState::READY: return "READY";
        case SystemState::DRYING: return "DRYING";
        case SystemState::COOLDOWN: return "COOLDOWN";
        case SystemState::STOPPED: return "STOPPED";
        case SystemState::FAULT_STOPPED: return "FAULT_STOPPED";
    }
    return "UNKNOWN";
}

String StateMachine::getStatusStreamName() const {
    switch (current_state_) {
        case SystemState::BOOT: return "boot";
        case SystemState::WIFI_CONNECT: return "wifi_connect";
        case SystemState::HOTSPOT: return "hotspot";
        case SystemState::READY: return "ready";
        case SystemState::DRYING: return "drying";
        case SystemState::COOLDOWN: return "cooldown";
        case SystemState::STOPPED: return "stopped";
        case SystemState::FAULT_STOPPED: return "fault_stopped";
    }
    return "unknown";
}

String StateMachine::stopReasonToString(DryingStopReason reason) {
    switch (reason) {
        case DryingStopReason::RUNNING: return "running";
        case DryingStopReason::COMPLETED: return "completed";
        case DryingStopReason::USER_STOPPED: return "user_stopped";
        case DryingStopReason::HUMIDITY_REACHED: return "humidity_reached";
        case DryingStopReason::MAX_TIME: return "max_time";
        case DryingStopReason::SAFETY_CUTOFF: return "safety_cutoff";
        case DryingStopReason::SENSOR_ERROR: return "sensor_error";
        case DryingStopReason::THERMAL_RUNAWAY: return "thermal_runaway";
    }
    return "unknown";
}

bool StateMachine::canTransition(SystemState from, SystemState to) const {
    uint8_t f = static_cast<uint8_t>(from);
    uint8_t t = static_cast<uint8_t>(to);
    if (f >= 8 || t >= 8) return false;
    return valid_transitions[f][t];
}

bool StateMachine::transitionTo(SystemState new_state) {
    if (!canTransition(current_state_, new_state)) {
        return false;
    }
    
    previous_state_ = current_state_;
    current_state_ = new_state;
    state_enter_time_ = millis();
    
    if (state_change_cb_) {
        state_change_cb_(previous_state_, new_state);
    }
    
    return true;
}

bool StateMachine::startDrying(const DryingSession& session) {
    if (current_state_ != SystemState::READY && current_state_ != SystemState::STOPPED) {
        return false;
    }
    
    static uint32_t next_session_id = 1;

    current_session_ = session;
    current_session_.session_id = next_session_id++;
    current_session_.status = SystemState::DRYING;
    current_session_.start_timestamp = millis();
    current_session_.stop_reason = DryingStopReason::RUNNING;
    
    return transitionTo(SystemState::DRYING);
}

bool StateMachine::beginCooldown(DryingStopReason reason) {
    if (current_state_ != SystemState::DRYING) {
        return false;
    }
    pending_cooldown_reason_ = reason;
    current_session_.status = SystemState::COOLDOWN;
    return transitionTo(SystemState::COOLDOWN);
}

bool StateMachine::completeCooldown() {
    if (current_state_ != SystemState::COOLDOWN) {
        return false;
    }
    current_session_.stop_reason = pending_cooldown_reason_;
    current_session_.status = SystemState::STOPPED;
    current_session_.elapsed_sec = (millis() - current_session_.start_timestamp) / 1000;
    return transitionTo(SystemState::STOPPED);
}

bool StateMachine::stopDrying(DryingStopReason reason) {
    if (current_state_ != SystemState::DRYING && current_state_ != SystemState::COOLDOWN) {
        return false;
    }
    
    current_session_.stop_reason = reason;
    current_session_.status = SystemState::STOPPED;
    current_session_.elapsed_sec = (millis() - current_session_.start_timestamp) / 1000;
    
    return transitionTo(SystemState::STOPPED);
}

bool StateMachine::updateDryingProgress(float current_temp, float current_humidity,
                                         float heater_power_pct, bool heater_on,
                                         float fan_power_pct, bool fan_on) {
    if (current_state_ != SystemState::DRYING && current_state_ != SystemState::COOLDOWN) {
        return false;
    }
    
    current_session_.elapsed_sec = (millis() - current_session_.start_timestamp) / 1000;
    current_session_.remaining_sec = (current_session_.max_duration_min * 60) - current_session_.elapsed_sec;
    current_session_.current_temp_c = current_temp;
    current_session_.current_humidity_pct = current_humidity;
    current_session_.heater_power_pct = heater_power_pct;
    current_session_.heater_on = heater_on;
    current_session_.exhaust_fan_power_pct = fan_power_pct;
    current_session_.exhaust_fan_on = fan_on;
    
    if (session_update_cb_) {
        session_update_cb_(current_session_);
    }
    
    return true;
}

uint32_t StateMachine::getSessionUptime() const {
    if ((current_state_ == SystemState::DRYING || current_state_ == SystemState::COOLDOWN) &&
        current_session_.start_timestamp > 0) {
        return (millis() - current_session_.start_timestamp) / 1000;
    }
    return 0;
}

float StateMachine::getProgressPercent() const {
    if (current_state_ != SystemState::DRYING || current_session_.max_duration_min == 0) {
        return 0.0f;
    }
    
    uint32_t total_sec = current_session_.max_duration_min * 60;
    uint32_t elapsed = getSessionUptime();
    
    if (elapsed >= total_sec) return 100.0f;
    return (static_cast<float>(elapsed) / total_sec) * 100.0f;
}

} // namespace filament_dryer
