/**
 * DriverRegistry - Implementation
 * Registers all built-in drivers for sensors, actuators, displays, and control algorithms
 */
#include "DriverRegistry.hpp"

namespace filament_dryer {

DriverRegistry& DriverRegistry::instance() {
    static DriverRegistry registry;
    return registry;
}

void DriverRegistry::registerSensor(const String& type, SensorFactory factory) {
    sensor_factories_[type] = std::move(factory);
}

void DriverRegistry::registerActuator(const String& type, ActuatorFactory factory) {
    actuator_factories_[type] = std::move(factory);
}

void DriverRegistry::registerDisplay(const String& type, DisplayFactory factory) {
    display_factories_[type] = std::move(factory);
}

void DriverRegistry::registerControl(const String& type, ControlFactory factory) {
    control_factories_[type] = std::move(factory);
}

ISensorDriver* DriverRegistry::createSensor(const String& type, const JsonObject& config) const {
    auto it = sensor_factories_.find(type);
    return it != sensor_factories_.end() ? it->second(config) : nullptr;
}

IActuatorDriver* DriverRegistry::createActuator(const String& type, const JsonObject& config) const {
    auto it = actuator_factories_.find(type);
    return it != actuator_factories_.end() ? it->second(config) : nullptr;
}

IDisplayDriver* DriverRegistry::createDisplay(const String& type, const JsonObject& config) const {
    auto it = display_factories_.find(type);
    return it != display_factories_.end() ? it->second(config) : nullptr;
}

IControlAlgorithm* DriverRegistry::createControl(const String& type, const JsonObject& config) const {
    auto it = control_factories_.find(type);
    return it != control_factories_.end() ? it->second(config) : nullptr;
}

std::vector<String> DriverRegistry::listSensors() const {
    std::vector<String> types;
    for (const auto& entry : sensor_factories_) types.push_back(entry.first);
    return types;
}

std::vector<String> DriverRegistry::listActuators() const {
    std::vector<String> types;
    for (const auto& entry : actuator_factories_) types.push_back(entry.first);
    return types;
}

std::vector<String> DriverRegistry::listDisplays() const {
    std::vector<String> types;
    for (const auto& entry : display_factories_) types.push_back(entry.first);
    return types;
}

std::vector<String> DriverRegistry::listControls() const {
    std::vector<String> types;
    for (const auto& entry : control_factories_) types.push_back(entry.first);
    return types;
}

bool DriverRegistry::hasSensor(const String& type) const {
    return sensor_factories_.find(type) != sensor_factories_.end();
}

bool DriverRegistry::hasActuator(const String& type) const {
    return actuator_factories_.find(type) != actuator_factories_.end();
}

bool DriverRegistry::hasDisplay(const String& type) const {
    return display_factories_.find(type) != display_factories_.end();
}

bool DriverRegistry::hasControl(const String& type) const {
    return control_factories_.find(type) != control_factories_.end();
}

void DriverRegistry::registerBuiltins() {
    // Built-in registration intentionally deferred until each driver family
    // exposes a stable constructor/factory surface.
}

} // namespace filament_dryer