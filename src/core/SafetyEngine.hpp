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
    SPI_BUS_ERROR,
    ACTUATOR_FAULT,
    SENSOR_RATE_OF_CHANGE,
    NVS_CORRUPT,
    WATCHDOG_RESET
};

struct SafetyConfig {
    float hard_temp_limit_c = 80.0f;
    float min_temp_c = -10.0f;
    float max_temp_c = 85.0f;
    float max_temp_rate_c_per_sec = 2.0f;
    uint8_t max_heater_power_pct = 100;
    uint16_t sensor_timeout_ms = 600;
    uint16_t thermal_runaway_time_sec = 45;
    float thermal_runaway_temp_rise = 0.5f;
    uint16_t cooldown_fan_duration_sec = 30;
    bool watchdog_enabled = true;
    uint32_t watchdog_timeout_ms = 8000;
    int8_t i2c_sda_pin = 21;
    int8_t i2c_scl_pin = 22;
};

class SafetyEngine {
public:
    SafetyEngine();
    ~SafetyEngine();

    bool begin(const SafetyConfig& config = SafetyConfig());

    bool checkSafety(float chamber_temp, float target_temp,
                     uint8_t heater_power_pct, bool heater_on,
                     bool sensor_connected);

    bool validateSensorReading(float chamber_temp, uint32_t now_ms);
    bool detectAndRecoverI2CBusLockup();
    using SpiRecoveryCallback = bool(*)();
    void setSpiRecoveryCallback(SpiRecoveryCallback cb) { spi_recovery_cb_ = cb; }

    bool detectAndRecoverSpiBusError(bool bus_error);
    // measured_from_feedback: only compare commanded vs measured when true (real sense/open-loop).
    // overcurrent: only from current-sense or open-loop trip — never from emergency-stop flag.
    bool checkActuatorFault(float commanded_power_pct, float measured_power_pct,
                            bool overcurrent = false, bool measured_from_feedback = false);

    void emergencyStop(FaultCode fault, const String& message);

    bool isFaulted() const { return faulted_; }
    FaultCode getLastFault() const { return last_fault_; }
    String getLastFaultMessage() const { return fault_message_; }

    void clearFault();

    void feedWatchdog();
    void enableWatchdog(bool enable);

    void setConfig(const SafetyConfig& config) { config_ = config; }
    const SafetyConfig& getConfig() const { return config_; }

    using FaultCallback = void(*)(FaultCode fault, const String& message);
    using EmergencyShutdownCallback = void(*)();
    void setFaultCallback(FaultCallback cb) { fault_cb_ = cb; }
    void setEmergencyShutdownCallback(EmergencyShutdownCallback cb) { emergency_cb_ = cb; }
    void attachCurrentTaskToWatchdog();

private:
    SafetyConfig config_;
    bool initialized_ = false;
    bool faulted_ = false;
    FaultCode last_fault_ = FaultCode::NONE;
    String fault_message_;

    uint8_t sensor_fail_count_ = 0;
    uint32_t sensor_disconnect_since_ms_ = 0;
    bool sensor_disconnect_tracking_ = false;
    bool last_sensor_connected_ = true;

    uint32_t high_power_start_time_ = 0;
    float last_temp_check_ = 0;
    uint32_t temp_check_time_ = 0;
    bool heater_was_high_power_ = false;

    float last_validated_temp_ = NAN;
    uint32_t last_validated_temp_ms_ = 0;
    bool has_last_validated_temp_ = false;

    bool watchdog_active_ = false;
    bool watchdog_task_attached_ = false;
    FaultCallback fault_cb_ = nullptr;
    EmergencyShutdownCallback emergency_cb_ = nullptr;
    SpiRecoveryCallback spi_recovery_cb_ = nullptr;

    void triggerFault(FaultCode fault, const String& message);
    void executeEmergencyShutdown();
    bool checkThermalRunaway(float chamber_temp, uint8_t heater_power_pct);
    void resetThermalRunawayTimer();
};

}  // namespace filament_dryer
