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
 * HardwareConfigParser - Generic hardware configuration parser and validator
 * Parses and validates the Klipper-style JSON configuration for sensors, actuators, displays, and control
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <string>
#include <map>

#include "ConfigManager.hpp"

namespace filament_dryer {

class HardwareConfigParser {
public:
    struct ValidationResult {
        bool valid = true;
        std::vector<String> errors;
        std::vector<String> warnings;
    };

    // Configuration schemas (embedded for validation)
    static constexpr const char* SENSOR_TYPES[] = {
        "sht3x", "sht30", "sht31", "dht22", "dht11", "am2302",
        "ds18b20", "ds18s20", "ntc", "thermistor",
        "bme280", "bmp280", "aht20", "aht10", "custom"
    };

    static constexpr const char* ACTUATOR_TYPES[] = {
        "mosfet_pwm", "ssr", "fan_pwm", "fan_digital",
        "shared_mosfet", "stepper", "servo", "gpio", "custom"
    };

    static constexpr const char* ACTUATOR_ROLES[] = {
        "heater", "fan", "custom"
    };

    // "custom" is recognized but rejected until a plugin registers a factory
    static constexpr const char* CONTROL_ALGORITHMS[] = {
        "pid", "bang_bang", "pwm_feedforward"
    };

    static constexpr const char* DISPLAY_DRIVERS[] = {
        "none", "ssd1306", "sh1106", "st7789", "ili9341",
        "st7735", "gc9a01", "ili9488", "hd44780", "nextion", "auto"
    };

    static constexpr const char* BUS_TYPES[] = {
        "i2c", "spi", "onewire", "uart", "adc"
    };

    HardwareConfigParser();
    ~HardwareConfigParser();

    // Parse and validate full hardware configuration
    ValidationResult parse(const JsonObject& config, 
                           SensorConfig& sensor_config,
                           ActuatorConfig& actuator_config,
                           DisplayConfig& display_config,
                           ControlConfig& control_config);

    // Parse individual sections
    ValidationResult parseSensors(const JsonArray& sensors, SensorConfig& config);
    ValidationResult parseActuators(const JsonArray& actuators, ActuatorConfig& config);
    ValidationResult parseDisplay(const JsonObject& display, DisplayConfig& config);
    ValidationResult parseControl(const JsonObject& control, ControlConfig& config);

    // Validate individual sensor
    ValidationResult validateSensor(const JsonObject& sensor);
    // Validate individual actuator
    ValidationResult validateActuator(const JsonObject& actuator);
    // Validate display config
    ValidationResult validateDisplay(const JsonObject& display);
    // Validate control config
    ValidationResult validateControl(const JsonObject& control);

    // Validation helpers
    bool isValidSensorType(const String& type) const;
    bool isValidActuatorType(const String& type) const;
    bool isValidActuatorRole(const String& role) const;
    bool isValidControlAlgorithm(const String& algorithm) const;
    bool isValidDisplayDriver(const String& driver) const;
    bool isValidBusType(const String& type) const;
    bool isValidGPIOPin(int pin) const;
    bool isValidOutputGPIOPin(int pin) const;
    bool isValidI2CAddress(int address) const;

    // Pin conflict detection
    ValidationResult checkPinConflicts(const SensorConfig& sensor,
                                       const ActuatorConfig& actuator,
                                       const DisplayConfig& display);

private:
    // Validation utilities
    void addError(ValidationResult& result, const String& message);
    void addWarning(ValidationResult& result, const String& message);
    bool isTypeInArray(const String& type, const char* arr[], size_t count) const;
};

// Convenience function
HardwareConfigParser::ValidationResult parseHardwareConfig(const String& json_string,
                                                           SensorConfig& sensor,
                                                           ActuatorConfig& actuator,
                                                           DisplayConfig& display,
                                                           ControlConfig& control);

} // namespace filament_dryer