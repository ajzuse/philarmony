/**
 * DriverRegistry - Implementation
 */
#include "DriverRegistry.hpp"

#include "../control/BangBangControl.hpp"
#include "../control/PIDControl.hpp"
#include "../control/PWMFeedforwardControl.hpp"
#include "../drivers/actuators/FanActuator.hpp"
#include "../drivers/actuators/GPIOActuator.hpp"
#include "../drivers/actuators/MosfetActuator.hpp"
#include "../drivers/actuators/ServoActuator.hpp"
#include "../drivers/actuators/SSRActuator.hpp"
#include "../drivers/actuators/SharedMosfetActuator.hpp"
#include "../drivers/actuators/StepperActuator.hpp"
#include "../drivers/display/GC9A01Display.hpp"
#include "../drivers/display/HD44780Display.hpp"
#include "../drivers/display/ILI9341Display.hpp"
#include "../drivers/display/ILI9488Display.hpp"
#include "../drivers/display/NextionDisplay.hpp"
#include "../drivers/display/SH1106Display.hpp"
#include "../drivers/display/SSD1306Display.hpp"
#include "../drivers/display/ST7735Display.hpp"
#include "../drivers/display/ST7789Display.hpp"
#include "../drivers/sensors/AHT20Sensor.hpp"
#include "../drivers/sensors/BME280Sensor.hpp"
#include "../drivers/sensors/CustomSensor.hpp"
#include "../drivers/sensors/DHT22Sensor.hpp"
#include "../drivers/sensors/DS18B20Sensor.hpp"
#include "../drivers/sensors/NTCSensor.hpp"
#include "../drivers/sensors/SHT31Sensor.hpp"
#include "../drivers/sensors/SHT3xSensor.hpp"

namespace filament_dryer {

namespace {

template <typename T>
ISensorDriver* makeSensor(const JsonObject& config) {
    auto* driver = new T();
    if (!driver->begin(config)) {
        delete driver;
        return nullptr;
    }
    return driver;
}

template <typename T>
IActuatorDriver* makeActuator(const JsonObject& config) {
    auto* driver = new T();
    if (!driver->begin(config)) {
        delete driver;
        return nullptr;
    }
    return driver;
}

template <typename T>
IDisplayDriver* makeDisplay(const JsonObject& config) {
    auto* driver = new T();
    if (!driver->begin(config)) {
        delete driver;
        return nullptr;
    }
    return driver;
}

template <typename T>
IControlAlgorithm* makeControl(const JsonObject& config) {
    auto* algorithm = new T(config);
    if (!algorithm->begin(config)) {
        delete algorithm;
        return nullptr;
    }
    return algorithm;
}

}  // namespace

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
    if (!sensor_factories_.empty()) {
        return;
    }

    registerSensor("sht3x", makeSensor<SHT3xSensor>);
    registerSensor("sht31", makeSensor<SHT31Sensor>);
    registerSensor("sht30", makeSensor<SHT31Sensor>);
    registerSensor("dht22", makeSensor<DHT22Sensor>);
    registerSensor("ds18b20", makeSensor<DS18B20Sensor>);
    registerSensor("ntc", makeSensor<NTCSensor>);
    registerSensor("bme280", makeSensor<BME280Sensor>);
    registerSensor("aht20", makeSensor<AHT20Sensor>);
    registerSensor("custom", makeSensor<CustomSensor>);

    registerActuator("mosfet_pwm", makeActuator<MosfetActuator>);
    registerActuator("mosfet_aod4184", makeActuator<MosfetActuator>);
    registerActuator("fan_pwm", makeActuator<FanActuator>);
    registerActuator("ssr", makeActuator<SSRActuator>);
    registerActuator("gpio", makeActuator<GPIOActuator>);
    registerActuator("servo", makeActuator<ServoActuator>);
    registerActuator("stepper", makeActuator<StepperActuator>);
    registerActuator("shared_mosfet", makeActuator<SharedMosfetActuator>);

    registerDisplay("ssd1306", makeDisplay<SSD1306Display>);
    registerDisplay("sh1106", makeDisplay<SH1106Display>);
    registerDisplay("st7789", makeDisplay<ST7789Display>);
    registerDisplay("st7735", makeDisplay<ST7735Display>);
    registerDisplay("ili9341", makeDisplay<ILI9341Display>);
    registerDisplay("gc9a01", makeDisplay<GC9A01Display>);
    registerDisplay("ili9488", makeDisplay<ILI9488Display>);
    registerDisplay("hd44780", makeDisplay<HD44780Display>);
    registerDisplay("nextion", makeDisplay<NextionDisplay>);

    registerControl("pid", makeControl<PIDControl>);
    registerControl("bang_bang", makeControl<BangBangControl>);
    registerControl("pwm_feedforward", makeControl<PWMFeedforwardControl>);
}

}  // namespace filament_dryer
