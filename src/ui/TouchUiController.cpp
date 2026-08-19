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

#include "TouchUiController.hpp"

namespace filament_dryer {

namespace {

template <typename Machine>
auto pauseMachine(Machine& machine, int)
    -> decltype(machine.pauseDrying(), bool()) {
    return machine.pauseDrying();
}

template <typename Machine>
bool pauseMachine(Machine&, long) {
    return false;
}

template <typename Machine>
auto resumeMachine(Machine& machine, int)
    -> decltype(machine.resumeDrying(), bool()) {
    return machine.resumeDrying();
}

template <typename Machine>
bool resumeMachine(Machine&, long) {
    return false;
}

template <typename State, typename Machine>
auto isPausedMachine(const Machine& machine, int)
    -> decltype(State::PAUSED, bool()) {
    return machine.getState() == State::PAUSED;
}

template <typename State, typename Machine>
bool isPausedMachine(const Machine&, long) {
    return false;
}

template <typename Reason>
auto pauseTimeoutReason(int) -> decltype(Reason::PAUSE_TIMEOUT) {
    return Reason::PAUSE_TIMEOUT;
}

template <typename Reason>
Reason pauseTimeoutReason(long) {
    // Compatibility until StateMachine adds the dedicated reason.
    return Reason::USER_STOPPED;
}

} // namespace

TouchUiController::TouchUiController(StateMachine& state_machine,
                                     ProfileManager& profile_manager,
                                     ConfigManager& config_manager)
    : state_machine_(state_machine),
      profile_manager_(profile_manager),
      config_manager_(config_manager) {}

bool TouchUiController::startFromProfile(const String& profile_id) {
    JsonDocument payload;
    payload["profile_id"] = profile_id;

    DryingSession session;
    if (!profile_manager_.buildSessionFromRequest(payload.as<JsonObject>(), session)) {
        return false;
    }
    if (!state_machine_.startDrying(session)) {
        return false;
    }

    pause_timer_active_ = false;
    broadcastStatus();
    return true;
}

bool TouchUiController::startCustom(float temperature_c, float humidity_pct,
                                    uint16_t duration_min) {
    JsonDocument payload;
    payload["target_temp_c"] = temperature_c;
    payload["target_humidity_pct"] = humidity_pct;
    payload["max_duration_min"] = duration_min;

    DryingSession session;
    if (!profile_manager_.buildSessionFromRequest(payload.as<JsonObject>(), session)) {
        return false;
    }
    if (!state_machine_.startDrying(session)) {
        return false;
    }

    pause_timer_active_ = false;
    broadcastStatus();
    return true;
}

bool TouchUiController::pause() {
    if (!pauseMachine(state_machine_, 0)) {
        return false;
    }

    pause_started_ms_ = millis();
    pause_timer_active_ = true;
    broadcastStatus();
    return true;
}

bool TouchUiController::resume() {
    if (!resumeMachine(state_machine_, 0)) {
        return false;
    }

    pause_timer_active_ = false;
    broadcastStatus();
    return true;
}

bool TouchUiController::stop() {
    if (!state_machine_.stopDrying(DryingStopReason::USER_STOPPED)) {
        return false;
    }

    pause_timer_active_ = false;
    broadcastStatus();
    return true;
}

bool TouchUiController::applyTargets(float temperature_c, float humidity_pct) {
    const bool paused =
        isPausedMachine<SystemState>(state_machine_, 0);
    if (!state_machine_.isDrying() && !paused) {
        return false;
    }

    DryingSession& session = state_machine_.getCurrentSession();
    if (!config_manager_.validateStartParams(
            temperature_c, session.max_duration_min, humidity_pct, true)) {
        return false;
    }

    session.target_temp_c = temperature_c;
    session.target_humidity_pct = humidity_pct;
    broadcastStatus();
    return true;
}

void TouchUiController::tick() {
    const bool paused =
        isPausedMachine<SystemState>(state_machine_, 0);
    if (!paused) {
        pause_timer_active_ = false;
        return;
    }

    if (!pause_timer_active_) {
        pause_started_ms_ = millis();
        pause_timer_active_ = true;
        return;
    }

    if (millis() - pause_started_ms_ < kPauseTimeoutMs) {
        return;
    }

    if (state_machine_.stopDrying(
            pauseTimeoutReason<DryingStopReason>(0))) {
        pause_timer_active_ = false;
        broadcastStatus();
    }
}

void TouchUiController::broadcastStatus() {
    if (!broadcast_callback_) {
        return;
    }

    const DryingSession& session = state_machine_.getCurrentSession();
    JsonDocument status;
    status["topic"] = "status/update";
    status["ui_source"] = getUiSource();
    status["status"] = state_machine_.getStatusStreamName();
    status["paused"] = isPausedMachine<SystemState>(state_machine_, 0);
    status["session_id"] = session.session_id;
    status["profile_id"] = session.profile_id;
    status["target_temp_c"] = session.target_temp_c;
    status["target_humidity_pct"] = session.target_humidity_pct;
    status["max_duration_min"] = session.max_duration_min;
    status["elapsed_time_sec"] = session.elapsed_sec;
    status["remaining_time_sec"] = session.remaining_sec;
    broadcast_callback_(status.as<JsonObject>());
}

} // namespace filament_dryer
