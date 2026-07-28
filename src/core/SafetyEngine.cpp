/**
 * SafetyEngine - Implementation
 */
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
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
    last_validated_temp_ = NAN;
    last_validated_temp_ms_ = 0;

    if (config_.watchdog_enabled) {
        const uint32_t timeout_sec = (config_.watchdog_timeout_ms + 999U) / 1000U;
        esp_err_t err = esp_task_wdt_init(timeout_sec > 0 ? timeout_sec : 1U, true);
        if (err == ESP_OK) {
            watchdog_active_ = true;
        }
    }

    return true;
}

bool SafetyEngine::validateSensorReading(float chamber_temp, uint32_t now_ms) {
    if (isnan(chamber_temp)) {
        return false;
    }

    if (chamber_temp < config_.min_temp_c || chamber_temp > config_.max_temp_c) {
        triggerFault(FaultCode::OVER_TEMPERATURE,
                     String("Sensor reading out of range: ") + chamber_temp + "C");
        return false;
    }

    if (!isnan(last_validated_temp_) && last_validated_temp_ms_ > 0) {
        const float dt_sec = (now_ms - last_validated_temp_ms_) / 1000.0f;
        if (dt_sec > 0.0f) {
            const float rate = fabsf(chamber_temp - last_validated_temp_) / dt_sec;
            if (rate > config_.max_temp_rate_c_per_sec) {
                triggerFault(FaultCode::ACTUATOR_FAULT,
                             String("Temperature rate-of-change too high: ") + rate + "C/s");
                return false;
            }
        }
    }

    last_validated_temp_ = chamber_temp;
    last_validated_temp_ms_ = now_ms;
    return true;
}

bool SafetyEngine::detectAndRecoverI2CBusLockup() {
    if (config_.i2c_sda_pin < 0 || config_.i2c_scl_pin < 0) {
        return true;
    }

    pinMode(config_.i2c_sda_pin, INPUT_PULLUP);
    if (digitalRead(config_.i2c_sda_pin) != LOW) {
        return true;
    }

    pinMode(config_.i2c_scl_pin, OUTPUT);
    for (int i = 0; i < 9; ++i) {
        digitalWrite(config_.i2c_scl_pin, HIGH);
        delayMicroseconds(5);
        digitalWrite(config_.i2c_scl_pin, LOW);
        delayMicroseconds(5);
    }
    digitalWrite(config_.i2c_scl_pin, HIGH);

    Wire.begin(config_.i2c_sda_pin, config_.i2c_scl_pin);

    pinMode(config_.i2c_sda_pin, INPUT_PULLUP);
    if (digitalRead(config_.i2c_sda_pin) == LOW) {
        triggerFault(FaultCode::I2C_BUS_LOCKUP, "I2C SDA stuck LOW after recovery");
        return false;
    }
    return true;
}

bool SafetyEngine::detectAndRecoverSpiBusError(bool bus_error) {
    if (!bus_error) {
        return true;
    }
    triggerFault(FaultCode::SPI_BUS_ERROR, "SPI bus error detected");
    return false;
}

bool SafetyEngine::checkActuatorFault(float commanded_power_pct, float measured_power_pct,
                                      bool overcurrent) {
    if (overcurrent) {
        triggerFault(FaultCode::ACTUATOR_FAULT, "Actuator overcurrent detected");
        return false;
    }

    if (fabsf(commanded_power_pct - measured_power_pct) > 15.0f && commanded_power_pct > 5.0f) {
        triggerFault(FaultCode::ACTUATOR_FAULT, "PWM output mismatch");
        return false;
    }
    return true;
}

bool SafetyEngine::checkSafety(float chamber_temp, float target_temp,
                               uint8_t heater_power_pct, bool heater_on,
                               bool sensor_connected) {
    (void)target_temp;
    if (!initialized_) return true;
    if (faulted_) return false;

    if (!sensor_connected) {
        sensor_fail_count_++;
        if (sensor_fail_count_ >= 3) {
            triggerFault(FaultCode::SENSOR_DISCONNECT,
                         "Sensor disconnected for >600ms");
            return false;
        }
    } else {
        sensor_fail_count_ = 0;
        if (!isnan(chamber_temp) && !validateSensorReading(chamber_temp, millis())) {
            return false;
        }
    }
    last_sensor_connected_ = sensor_connected;

    if (!isnan(chamber_temp) && chamber_temp >= config_.hard_temp_limit_c) {
        triggerFault(FaultCode::OVER_TEMPERATURE,
                     String("Chamber temperature ") + chamber_temp + "C >= " +
                         config_.hard_temp_limit_c + "C");
        return false;
    }

    if (heater_on && heater_power_pct > 80) {
        if (!heater_was_high_power_) {
            high_power_start_time_ = millis();
            last_temp_check_ = chamber_temp;
            temp_check_time_ = millis();
            heater_was_high_power_ = true;
        } else if (millis() - temp_check_time_ >= 10000) {
            float temp_rise = chamber_temp - last_temp_check_;
            uint32_t elapsed_sec = (millis() - high_power_start_time_) / 1000;

            if (elapsed_sec >= config_.thermal_runaway_time_sec &&
                temp_rise < config_.thermal_runaway_temp_rise) {
                triggerFault(FaultCode::THERMAL_RUNAWAY,
                             String("Heater >80% for ") + elapsed_sec +
                                 "s with only " + temp_rise + "C rise");
                return false;
            }

            last_temp_check_ = chamber_temp;
            temp_check_time_ = millis();
        }
    } else {
        resetThermalRunawayTimer();
    }

    if (watchdog_active_) {
        esp_task_wdt_reset();
    }

    return true;
}

void SafetyEngine::emergencyStop(FaultCode fault, const String& message) {
    triggerFault(fault, message);
}

void SafetyEngine::triggerFault(FaultCode fault, const String& message) {
    if (faulted_) return;

    faulted_ = true;
    last_fault_ = fault;
    fault_message_ = message;

    executeEmergencyShutdown();

    if (fault_cb_) {
        fault_cb_(fault, message);
    }
}

void SafetyEngine::executeEmergencyShutdown() {
    if (emergency_cb_) {
        emergency_cb_();
    }
}

void SafetyEngine::attachCurrentTaskToWatchdog() {
    if (!watchdog_active_ || watchdog_task_attached_) {
        return;
    }
    if (esp_task_wdt_add(nullptr) == ESP_OK) {
        watchdog_task_attached_ = true;
    }
}

void SafetyEngine::clearFault() {
    faulted_ = false;
    last_fault_ = FaultCode::NONE;
    fault_message_ = "";
    sensor_fail_count_ = 0;
    last_validated_temp_ = NAN;
    last_validated_temp_ms_ = 0;
    resetThermalRunawayTimer();
}

void SafetyEngine::feedWatchdog() {
    if (watchdog_active_) {
        esp_task_wdt_reset();
    }
}

void SafetyEngine::enableWatchdog(bool enable) {
    if (enable && !watchdog_active_ && config_.watchdog_enabled) {
        const uint32_t timeout_sec = (config_.watchdog_timeout_ms + 999U) / 1000U;
        esp_task_wdt_init(timeout_sec > 0 ? timeout_sec : 1U, true);
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
    (void)chamber_temp;
    (void)heater_power_pct;
    return false;
}

}  // namespace filament_dryer
