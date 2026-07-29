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
 * DriverRegistry - Dynamic driver factory for sensors, actuators, displays, and control algorithms
 * Klipper-inspired object registry pattern for 100% configurable hardware
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <functional>
#include <memory>
#include <vector>

#include "drivers/interfaces/IDriverInterfaces.hpp"
#include "control/IControlAlgorithm.hpp"

namespace filament_dryer {

class DriverRegistry {
public:
    using SensorFactory = std::function<ISensorDriver*(const JsonObject&)>;
    using ActuatorFactory = std::function<IActuatorDriver*(const JsonObject&)>;
    using DisplayFactory = std::function<IDisplayDriver*(const JsonObject&)>;
    using ControlFactory = std::function<IControlAlgorithm*(const JsonObject&)>;

    // Singleton access
    static DriverRegistry& instance();

    // ===========================================
    // REGISTRATION METHODS (called at startup)
    // ===========================================
    
    // Register sensor driver by type string
    template<typename T>
    void registerSensor(const String& type) {
        sensor_factories_[type] = [](const JsonObject& config) -> ISensorDriver* {
            return new T(config);
        };
    }
    void registerSensor(const String& type, SensorFactory factory);

    // Register actuator driver by type string
    template<typename T>
    void registerActuator(const String& type) {
        actuator_factories_[type] = [](const JsonObject& config) -> IActuatorDriver* {
            return new T(config);
        };
    }
    void registerActuator(const String& type, ActuatorFactory factory);

    // Register display driver by type string
    template<typename T>
    void registerDisplay(const String& type) {
        display_factories_[type] = [](const JsonObject& config) -> IDisplayDriver* {
            return new T(config);
        };
    }
    void registerDisplay(const String& type, DisplayFactory factory);

    // Register control algorithm by type string
    template<typename T>
    void registerControl(const String& type) {
        control_factories_[type] = [](const JsonObject& config) -> IControlAlgorithm* {
            return new T(config);
        };
    }
    void registerControl(const String& type, ControlFactory factory);

    // ===========================================
    // CREATION METHODS (called at runtime)
    // ===========================================

    // Create sensor by type string
    ISensorDriver* createSensor(const String& type, const JsonObject& config) const;

    // Create actuator by type string
    IActuatorDriver* createActuator(const String& type, const JsonObject& config) const;

    // Create display by type string
    IDisplayDriver* createDisplay(const String& type, const JsonObject& config) const;

    // Create control algorithm by type string
    IControlAlgorithm* createControl(const String& type, const JsonObject& config) const;

    // List available drivers
    std::vector<String> listSensors() const;
    std::vector<String> listActuators() const;
    std::vector<String> listDisplays() const;
    std::vector<String> listControls() const;

    // Check if type is registered
    bool hasSensor(const String& type) const;
    bool hasActuator(const String& type) const;
    bool hasDisplay(const String& type) const;
    bool hasControl(const String& type) const;

    // Initialize - registers all built-in drivers
    void registerBuiltins();

private:
    DriverRegistry() = default;
    ~DriverRegistry() = default;
    DriverRegistry(const DriverRegistry&) = delete;
    DriverRegistry& operator=(const DriverRegistry&) = delete;

    std::map<String, std::function<ISensorDriver*(const JsonObject&)>> sensor_factories_;
    std::map<String, std::function<IActuatorDriver*(const JsonObject&)>> actuator_factories_;
    std::map<String, std::function<IDisplayDriver*(const JsonObject&)>> display_factories_;
    std::map<String, std::function<IControlAlgorithm*(const JsonObject&)>> control_factories_;

};

} // namespace filament_dryer