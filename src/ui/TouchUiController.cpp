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
                                     ConfigManager& config_manager,
                                     SafetyEngine* safety)
    : state_machine_(state_machine),
      profile_manager_(profile_manager),
      config_manager_(config_manager),
      safety_(safety) {}

bool TouchUiController::startFromProfile(const String& profile_id) {
    JsonDocument payload;
    payload["profile_id"] = profile_id;

    DryingSession session;
    if (!profile_manager_.buildSessionFromRequest(payload.as<JsonObject>(), session)) {
        return false;
    }
    if (!safetyAllows(session.target_temp_c)) {
        return false;
    }
    if (!state_machine_.startDrying(session)) {
        return false;
    }

    pause_timer_active_ = false;
    pause_timeout_pending_ = false;
    pause_timeout_reported_ = false;
    broadcastStatus();
    return true;
}

bool TouchUiController::startCustom(float temperature_c, float humidity_pct,
                                    uint16_t duration_min) {
    if (!safetyAllows(temperature_c)) {
        return false;
    }
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
    pause_timeout_pending_ = false;
    pause_timeout_reported_ = false;
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
    if (!safetyAllows(state_machine_.getCurrentSession().target_temp_c)) {
        return false;
    }
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

bool TouchUiController::applyTargets(float temperature_c, float humidity_pct,
                                     uint16_t duration_min) {
    if (!safetyAllows(temperature_c)) {
        return false;
    }
    const bool paused =
        isPausedMachine<SystemState>(state_machine_, 0);
    if (!state_machine_.isDrying() && !paused) {
        return false;
    }

    DryingSession& session = state_machine_.getCurrentSession();
    if (!config_manager_.validateStartParams(
            temperature_c, duration_min, humidity_pct, true)) {
        return false;
    }

    session.target_temp_c = temperature_c;
    session.target_humidity_pct = humidity_pct;
    session.max_duration_min = duration_min;
    const uint32_t total_sec = static_cast<uint32_t>(duration_min) * 60UL;
    session.remaining_sec =
        total_sec > session.elapsed_sec ? total_sec - session.elapsed_sec : 0;
    broadcastStatus();
    return true;
}

void TouchUiController::tick() {
    if (state_machine_.getState() == SystemState::STOPPED &&
        state_machine_.getCurrentSession().stop_reason ==
            DryingStopReason::PAUSE_TIMEOUT) {
        if (!pause_timeout_reported_) {
            pause_timeout_pending_ = true;
            pause_timeout_reported_ = true;
        }
        pause_timer_active_ = false;
        return;
    }

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
        pause_timeout_pending_ = true;
        pause_timeout_reported_ = true;
        broadcastStatus();
    }
}

std::vector<FilamentProfile> TouchUiController::listProfiles() const {
    return profile_manager_.listProfiles();
}

UISettings TouchUiController::getUiSettings() const {
    return config_manager_.getUiSettings();
}

TouchConfig TouchUiController::getTouchConfig() const {
    return config_manager_.getTouchConfig();
}

bool TouchUiController::applyUiSettings(const UISettings& settings) {
    const bool orientation_ok =
        settings.orientation == 0 || settings.orientation == 90 ||
        settings.orientation == 180 || settings.orientation == 270;
    const bool unit_ok =
        settings.temp_unit == "celsius" || settings.temp_unit == "fahrenheit";
    const bool language_ok =
        settings.language == "pt_br" || settings.language == "en_us";
    const bool sensitivity_ok =
        settings.touch_sensitivity == "low" ||
        settings.touch_sensitivity == "medium" ||
        settings.touch_sensitivity == "high";
    if (settings.brightness_pct < 10 || settings.brightness_pct > 100 ||
        settings.timeout_sec < 30 || settings.timeout_sec > 600 ||
        !orientation_ok || !unit_ok || !language_ok || !sensitivity_ok) {
        broadcastAck("config/ui/ack", false);
        return false;
    }
    const bool saved = config_manager_.setUiSettings(settings);
    broadcastAck("config/ui/ack", saved);
    return saved;
}

bool TouchUiController::applyTouchConfig(const TouchConfig& config) {
    const bool type_ok =
        config.controller_type == "auto" ||
        config.controller_type == "xpt2046" ||
        config.controller_type == "stmpe610" ||
        config.controller_type == "ft6236" ||
        config.controller_type == "gt911" ||
        config.controller_type == "cst816s" ||
        config.controller_type == "none";
    const bool sensitivity_ok =
        config.sensitivity == "low" || config.sensitivity == "medium" ||
        config.sensitivity == "high";
    const bool calibration_ok =
        config.calibration.x_max > config.calibration.x_min &&
        config.calibration.y_max > config.calibration.y_min;
    if (!type_ok || !sensitivity_ok || !calibration_ok) {
        broadcastAck("config/touch/ack", false);
        return false;
    }
    const bool saved = config_manager_.setTouchConfig(config);
    broadcastAck("config/touch/ack", saved);
    return saved;
}

void TouchUiController::requestWifiHotspot() {
    if (wifi_hotspot_callback_) {
        wifi_hotspot_callback_();
        broadcastAck("config/wifi/ack", true);
    } else {
        broadcastAck("config/wifi/ack", false);
    }
}

bool TouchUiController::consumePauseTimeoutEvent() {
    const bool pending = pause_timeout_pending_;
    pause_timeout_pending_ = false;
    return pending;
}

bool TouchUiController::safetyAllows(float temperature_c) const {
    if (!safety_) return true;
    return !safety_->isFaulted() &&
           temperature_c <= safety_->getConfig().hard_temp_limit_c;
}

void TouchUiController::broadcastAck(const char* topic, bool success) {
    if (!broadcast_callback_) return;
    JsonDocument ack;
    ack["topic"] = topic;
    ack["success"] = success;
    ack["ui_source"] = getUiSource();
    broadcast_callback_(ack.as<JsonObject>());
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
