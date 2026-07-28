/**
 * StateMachine - Implementation
 */
#include "StateMachine.hpp"

namespace filament_dryer {

constexpr bool StateMachine::valid_transitions[7][7];

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
        case SystemState::STOPPED: return "STOPPED";
        case SystemState::FAULT_STOPPED: return "FAULT_STOPPED";
    }
    return "UNKNOWN";
}

bool StateMachine::canTransition(SystemState from, SystemState to) const {
    uint8_t f = static_cast<uint8_t>(from);
    uint8_t t = static_cast<uint8_t>(to);
    if (f >= 7 || t >= 7) return false;
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
        state_change_cb_(previous_state_, current_state_);
    }
    
    return true;
}

bool StateMachine::startDrying(const DryingSession& session) {
    if (current_state_ != SystemState::READY && current_state_ != SystemState::STOPPED) {
        return false;
    }
    
    current_session_ = session;
    current_session_.status = SystemState::DRYING;
    current_session_.start_timestamp = millis();
    current_session_.stop_reason = DryingStopReason::RUNNING;
    
    return transitionTo(SystemState::DRYING);
}

bool StateMachine::stopDrying(DryingStopReason reason) {
    if (current_state_ != SystemState::DRYING) {
        return false;
    }
    
    current_session_.stop_reason = reason;
    current_session_.status = SystemState::STOPPED;
    current_session_.elapsed_sec = (millis() - current_session_.start_timestamp) / 1000;
    
    transitionTo(SystemState::STOPPED);
    return true;
}

bool StateMachine::updateDryingProgress(float current_temp, float current_humidity,
                                         float heater_power_pct, bool heater_on,
                                         float fan_power_pct, bool fan_on) {
    if (current_state_ != SystemState::DRYING) {
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
    if (current_state_ == SystemState::DRYING && current_session_.start_timestamp > 0) {
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