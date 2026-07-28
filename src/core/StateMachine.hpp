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
    STOPPED,
    FAULT_STOPPED
};

enum class DryingStopReason {
    RUNNING,
    COMPLETED,
    USER_STOPPED,
    HUMIDITY_REACHED,
    MAX_TIME,
    SAFETY_CUTOFF,
    SENSOR_ERROR,
    THERMAL_RUNAWAY
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
    StateMachine();
    ~StateMachine();
    
    bool begin();
    
    // State access
    SystemState getState() const { return current_state_; }
    String getStateName() const;
    
    // State transitions
    bool transitionTo(SystemState new_state);
    bool canTransition(SystemState from, SystemState to) const;
    
    // Drying session management
    bool startDrying(const DryingSession& session);
    bool stopDrying(DryingStopReason reason);
    bool updateDryingProgress(float current_temp, float current_humidity, 
                               float heater_power_pct, bool heater_on,
                               float fan_power_pct, bool fan_on);
    
    const DryingSession& getCurrentSession() const { return current_session_; }
    DryingSession& getCurrentSession() { return current_session_; }
    
    // Safety state checks
    bool isInFaultState() const { return current_state_ == SystemState::FAULT_STOPPED; }
    bool isDrying() const { return current_state_ == SystemState::DRYING; }
    bool isReady() const { return current_state_ == SystemState::READY; }
    
    // Time tracking
    uint32_t getSessionUptime() const;
    float getProgressPercent() const;
    
    // Callbacks
    using StateChangeCallback = void(*)(SystemState old_state, SystemState new_state);
    using SessionUpdateCallback = void(*)(const DryingSession& session);
    
    void setStateChangeCallback(StateChangeCallback cb) { state_change_cb_ = cb; }
    void setSessionUpdateCallback(SessionUpdateCallback cb) { session_update_cb_ = cb; }

private:
    SystemState current_state_ = SystemState::BOOT;
    SystemState previous_state_ = SystemState::BOOT;
    DryingSession current_session_;
    uint32_t state_enter_time_ = 0;
    
    StateChangeCallback state_change_cb_ = nullptr;
    SessionUpdateCallback session_update_cb_ = nullptr;
    
    // State transition validation
    static constexpr bool valid_transitions[7][7] = {
        // BOOT, WIFI, HOTSPOT, READY, DRYING, STOPPED, FAULT
        {false, true,  true,  false, false,  false,  false}, // BOOT
        {false, false, true,  true,  false,  false,  false}, // WIFI_CONNECT
        {false, true,  false, true,  false,  false,  false}, // HOTSPOT
        {false, false, true,  false, true,   false,  false}, // READY
        {false, false, false, true,  true,   true,   true},  // DRYING
        {false, false, true,  true,  false,  false,  false}, // STOPPED
        {false, false, true,  true,  false,  false,  false}  // FAULT_STOPPED
    };
};

} // namespace filament_dryer