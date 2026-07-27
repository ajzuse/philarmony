/**
 * SafetyEngine - Implementation
 * Thermal runaway protection, sensor monitoring, and hardware watchdog
 */
#include <Arduino.h>
#include "SafetyEngine.hpp"

namespace filament_dryer {

SafetyEngine::SafetyEngine() {}

SafetyEngine::~SafetyEngine() {
    if (watchdog_active_) {
        esp_task_wdt_deinit();
    }
}

bool SafetyEngine::begin(const SafetyConfig& config) {
    config_ = config;
    initialized_ = true;
    faulted_ = false;
    last_fault_ = FaultCode::NONE;
    fault_message_ = "";
    
    // Initialize hardware watchdog using esp_task_wdt (new API)
    if (config_.watchdog_enabled) {
        esp_task_wdt_config_t wdt_config = {
            .timeout_ms = config_.watchdog_timeout_ms,
            .idle_core_mask = (1 << 0) | (1 << 1), // Both cores
            .trigger_panic = true
        };
        esp_err_t err = esp_task_wdt_init(&wdt_config);
        if (err == ESP_OK) {
            watchdog_active_ = true;
        }
    }
    
    return true;
}

bool SafetyEngine::checkSafety(float chamber_temp, float target_temp,
                               uint8_t heater_power_pct, bool heater_on,
                               bool sensor_connected) {
    if (!initialized_) return true;
    if (faulted_) return false; // Stay in fault state until cleared
    
    // 1. Sensor connectivity check
    if (!sensor_connected) {
        sensor_fail_count_++;
        if (sensor_fail_count_ >= 3) { // 3 consecutive failures
            triggerFault(FaultCode::SENSOR_DISCONNECT, 
                        "Sensor disconnected for >600ms");
            return false;
        }
    } else {
        sensor_fail_count_ = 0;
    }
    last_sensor_connected_ = sensor_connected;
    
    // 2. Hard temperature limit (immediate hardware cutoff)
    if (chamber_temp >= config_.hard_temp_limit_c) {
        triggerFault(FaultCode::OVER_TEMPERATURE,
                    String("Chamber temperature ") + chamber_temp + "C >= " + config_.hard_temp_limit_c + "C");
        return false;
    }
    
    // 3. Thermal runaway detection
    if (heater_on && heater_power_pct > 80) {
        if (!heater_was_high_power_) {
            high_power_start_time_ = millis();
            last_temp_check_ = chamber_temp;
            temp_check_time_ = millis();
            heater_was_high_power_ = true;
        } else {
            // Check if enough time has passed for thermal runaway check
            if (millis() - temp_check_time_ >= 10000) { // Check every 10s
                float temp_rise = chamber_temp - last_temp_check_;
                uint32_t elapsed_sec = (millis() - high_power_start_time_) / 1000;
                
                if (elapsed_sec >= config_.thermal_runaway_time_sec && 
                    temp_rise < config_.thermal_runaway_temp_rise) {
                    triggerFault(FaultCode::THERMAL_RUNAWAY,
                                String("Heater >80% for ") + elapsed_sec + 
                                "s with only " + temp_rise + "C rise");
                    return false;
                }
                
                // Reset check window
                last_temp_check_ = chamber_temp;
                temp_check_time_ = millis();
            }
        }
    } else {
        resetThermalRunawayTimer();
    }
    
    // 4. Soft power limit enforcement
    if (heater_power_pct > config_.max_heater_power_pct) {
        // This is handled by the control loop, but we log it
    }
    
    // Feed watchdog if active
    if (watchdog_active_) {
        esp_task_wdt_reset();
    }
    
    return true;
}

void SafetyEngine::emergencyStop(FaultCode fault, const String& message) {
    triggerFault(fault, message);
}

void SafetyEngine::triggerFault(FaultCode fault, const String& message) {
    if (faulted_) return; // Already faulted
    
    faulted_ = true;
    last_fault_ = fault;
    fault_message_ = message;
    
    executeEmergencyShutdown();
    
    if (fault_cb_) {
        fault_cb_(fault, message);
    }
}

void SafetyEngine::executeEmergencyShutdown() {
    // This is called from the control loop context
    // The actual GPIO write is done by the actuator driver
    // We just set the fault state
}

void SafetyEngine::clearFault() {
    faulted_ = false;
    last_fault_ = FaultCode::NONE;
    fault_message_ = "";
    sensor_fail_count_ = 0;
    resetThermalRunawayTimer();
}

void SafetyEngine::feedWatchdog() {
    if (watchdog_active_) {
        esp_task_wdt_reset();
    }
}

void SafetyEngine::enableWatchdog(bool enable) {
    if (enable && !watchdog_active_ && config_.watchdog_enabled) {
        esp_task_wdt_config_t wdt_config = {
            .timeout_ms = config_.watchdog_timeout_ms,
            .idle_core_mask = (1 << 0) | (1 << 1), // Both cores
            .trigger_panic = true
        };
        esp_task_wdt_init(&wdt_config);
        watchdog_active_ = true;
    } else if (!enable && watchdog_active_) {
        esp_task_wdt_deinit();
        watchdog_active_ = false;
    }
}

void SafetyEngine::resetThermalRunawayTimer() {
    high_power_start_time_ = 0;
    last_temp_check_ = 0;
    temp_check_time_ = 0;
    heater_was_high_power_ = false;
}

bool SafetyEngine::checkThermalRunaway(float chamber_temp, uint8_t heater_power_pct) {
    // Handled in checkSafety
    return false;
}

} // namespace filament_dryer