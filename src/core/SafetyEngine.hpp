/**
 * SafetyEngine - Thermal runaway protection and hardware watchdog
 */
#pragma once

#include <Arduino.h>
#include <esp_task_wdt.h>

namespace filament_dryer {

enum class FaultCode {
    NONE,
    SENSOR_DISCONNECT,
    OVER_TEMPERATURE,
    THERMAL_RUNAWAY,
    I2C_BUS_LOCKUP,
    NVS_CORRUPT,
    WATCHDOG_RESET
};

struct SafetyConfig {
    float hard_temp_limit_c = 80.0f;         // Hard temperature cutoff
    uint8_t max_heater_power_pct = 100;      // Soft power limit
    uint16_t sensor_timeout_ms = 600;        // 3 failed reads * 200ms
    uint16_t thermal_runaway_time_sec = 45;  // Heater >80% for 45s without +0.5C rise
    float thermal_runaway_temp_rise = 0.5f;  // Minimum temp rise expected
    uint16_t cooldown_fan_duration_sec = 30; // Post-heat fan run
    bool watchdog_enabled = true;            // Hardware watchdog
    uint32_t watchdog_timeout_ms = 8000;     // 8 second watchdog
};

class SafetyEngine {
public:
    SafetyEngine();
    ~SafetyEngine();
    
    bool begin(const SafetyConfig& config = SafetyConfig());
    
    // Main safety check - call every control loop iteration
    bool checkSafety(float chamber_temp, float target_temp, 
                     uint8_t heater_power_pct, bool heater_on,
                     bool sensor_connected);
    
    // Immediate emergency stop
    void emergencyStop(FaultCode fault, const String& message);
    
    // Check if in fault state
    bool isFaulted() const { return faulted_; }
    FaultCode getLastFault() const { return last_fault_; }
    String getLastFaultMessage() const { return fault_message_; }
    
    // Reset fault state (only from external/user intervention)
    void clearFault();
    
    // Watchdog management
    void feedWatchdog();
    void enableWatchdog(bool enable);
    
    // Configuration
    void setConfig(const SafetyConfig& config) { config_ = config; }
    const SafetyConfig& getConfig() const { return config_; }
    
    // Callbacks
    using FaultCallback = void(*)(FaultCode fault, const String& message);
    void setFaultCallback(FaultCallback cb) { fault_cb_ = cb; }

private:
    SafetyConfig config_;
    bool initialized_ = false;
    bool faulted_ = false;
    FaultCode last_fault_ = FaultCode::NONE;
    String fault_message_;
    
    // Sensor monitoring
    uint8_t sensor_fail_count_ = 0;
    bool last_sensor_connected_ = true;
    
    // Thermal runaway detection
    uint32_t high_power_start_time_ = 0;
    float last_temp_check_ = 0;
    uint32_t temp_check_time_ = 0;
    bool heater_was_high_power_ = false;
    
    // Watchdog
    bool watchdog_active_ = false;
    
    FaultCallback fault_cb_ = nullptr;
    
    // Helper methods
    void triggerFault(FaultCode fault, const String& message);
    void executeEmergencyShutdown();
    bool checkThermalRunaway(float chamber_temp, uint8_t heater_power_pct);
    void resetThermalRunawayTimer();
};

} // namespace filament_dryer