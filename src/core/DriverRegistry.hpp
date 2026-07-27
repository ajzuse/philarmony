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

#include "drivers/interfaces/IDriverInterfaces.hpp"

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

    // Register actuator driver by type string
    template<typename T>
    void registerActuator(const String& type) {
        actuator_factories_[type] = [](const JsonObject& config) -> IActuatorDriver* {
            return new T(config);
        };
    }

    // Register display driver by type string
    template<typename T>
    void registerDisplay(const String& type) {
        display_factories_[type] = [](const JsonObject& config) -> IDisplayDriver* {
            return new T(config);
        };
    }

    // Register control algorithm by type string
    template<typename T>
    void registerControl(const String& type) {
        control_factories_[type] = [](const JsonObject& config) -> IControlAlgorithm* {
            return new T(config);
        };
    }

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
    void initializeBuiltins();

private:
    DriverRegistry() = default;
    ~DriverRegistry() = default;
    DriverRegistry(const DriverRegistry&) = delete;
    DriverRegistry& operator=(const DriverRegistry&) = delete;

    std::map<String, std::function<ISensorDriver*(const JsonObject&)>> sensor_factories_;
    std::map<String, std::function<IActuatorDriver*(const JsonObject&)>> actuator_factories_;
    std::map<String, std::function<IDisplayDriver*(const JsonObject&)>> display_factories_;
    std::map<String, std::function<IControlAlgorithm*(const JsonObject&)>> control_factories_;

    // Explicit template instantiations for common drivers
    template void registerSensor<DHT22Sensor>(const String&);
    template void registerSensor<DS18B20Sensor>(const String&);
    template void registerSensor<NTCSensor>(const String&);
    template void registerSensor<BME280Sensor>(const String&);
    template void registerSensor<AHT20Sensor>(const String&);
    template void registerSensor<CustomSensor>(const String&);

    template void registerActuator<MosfetActuator>(const String&);
    template void registerActuator<SSRActuator>(const String&);
    template void registerActuator<FanActuator>(const String&);
    template void registerActuator<StepperActuator>(const String&);
    template void registerActuator<ServoActuator>(const String&);
    template void registerActuator<GPIOActuator>(const String&);
    template void registerActuator<SharedMosfetActuator>(const String&);

    template void registerDisplay<SSD1306Display>(const String&);
    template void registerDisplay<ST7789Display>(const String&);
    template void registerDisplay<ILI9341Display>(const String&);
    template void registerDisplay<ST7735Display>(const String&);
    template void registerDisplay<GC9A01Display>(const String&);
    template void registerDisplay<ILI9488Display>(const String&);
    template void registerDisplay<HD44780Display>(const String&);
    template void registerDisplay<NextionDisplay>(const String&);

    template void registerControl<PIDControl>(const String&);
    template void registerControl<BangBangControl>(const String&);
    template void registerControl<PWMFeedforwardControl>(const String&);
};

} // namespace filament_dryer