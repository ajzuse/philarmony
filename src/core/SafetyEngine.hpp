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
    bool detectAndRecoverSpiBusError(bool bus_error);
    bool checkActuatorFault(float commanded_power_pct, float measured_power_pct,
                            bool overcurrent = false);

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
    void setFaultCallback(FaultCallback cb) { fault_cb_ = cb; }

private:
    SafetyConfig config_;
    bool initialized_ = false;
    bool faulted_ = false;
    FaultCode last_fault_ = FaultCode::NONE;
    String fault_message_;

    uint8_t sensor_fail_count_ = 0;
    bool last_sensor_connected_ = true;

    uint32_t high_power_start_time_ = 0;
    float last_temp_check_ = 0;
    uint32_t temp_check_time_ = 0;
    bool heater_was_high_power_ = false;

    float last_validated_temp_ = NAN;
    uint32_t last_validated_temp_ms_ = 0;

    bool watchdog_active_ = false;
    FaultCallback fault_cb_ = nullptr;

    void triggerFault(FaultCode fault, const String& message);
    void executeEmergencyShutdown();
    bool checkThermalRunaway(float chamber_temp, uint8_t heater_power_pct);
    void resetThermalRunawayTimer();
};

}  // namespace filament_dryer
