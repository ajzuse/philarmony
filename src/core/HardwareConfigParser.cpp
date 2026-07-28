/**
 * HardwareConfigParser - Implementation
 * Generic hardware configuration parser and validator
 */
#include "HardwareConfigParser.hpp"
#include "ConfigManager.hpp"
#include <map>

namespace filament_dryer {

using filament_dryer::SensorConfig;
using filament_dryer::ActuatorConfig;
using filament_dryer::DisplayConfig;
using filament_dryer::ControlConfig;

HardwareConfigParser::HardwareConfigParser() {}

HardwareConfigParser::~HardwareConfigParser() {}

HardwareConfigParser::ValidationResult HardwareConfigParser::parse(const JsonObject& config,
                                                                   SensorConfig& sensor_config,
                                                                   ActuatorConfig& actuator_config,
                                                                   DisplayConfig& display_config,
                                                                   ControlConfig& control_config) {
    ValidationResult result;

    // Parse sensors
    if (config.containsKey("sensors")) {
        JsonArray sensors = config["sensors"].as<JsonArray>();
        auto sensor_result = parseSensors(sensors, sensor_config);
        if (!sensor_result.valid) {
            result.valid = false;
            for (const auto& err : sensor_result.errors) {
                addError(result, "sensors: " + err);
            }
            for (const auto& warn : sensor_result.warnings) {
                addWarning(result, "sensors: " + warn);
            }
        }
    }

    // Parse actuators
    if (config.containsKey("actuators")) {
        JsonArray actuators = config["actuators"].as<JsonArray>();
        auto actuator_result = parseActuators(actuators, actuator_config);
        if (!actuator_result.valid) {
            result.valid = false;
            for (const auto& err : actuator_result.errors) {
                addError(result, "actuators: " + err);
            }
            for (const auto& warn : actuator_result.warnings) {
                addWarning(result, "actuators: " + warn);
            }
        }
    }

    // Parse display
    if (config.containsKey("display")) {
        JsonObject display = config["display"].as<JsonObject>();
        auto display_result = parseDisplay(display, display_config);
        if (!display_result.valid) {
            result.valid = false;
            for (const auto& err : display_result.errors) {
                addError(result, "display: " + err);
            }
            for (const auto& warn : display_result.warnings) {
                addWarning(result, "display: " + warn);
            }
        }
    }

    // Parse control
    if (config.containsKey("control")) {
        JsonObject control = config["control"].as<JsonObject>();
        auto control_result = parseControl(control, control_config);
        if (!control_result.valid) {
            result.valid = false;
            for (const auto& err : control_result.errors) {
                addError(result, "control: " + err);
            }
            for (const auto& warn : control_result.warnings) {
                addWarning(result, "control: " + warn);
            }
        }
    }

    // Check pin conflicts across all components
    if (result.valid) {
        auto conflict_result = checkPinConflicts(sensor_config, actuator_config, display_config);
        if (!conflict_result.valid) {
            result.valid = false;
            for (const auto& err : conflict_result.errors) {
                addError(result, "pin_conflicts: " + err);
            }
        }
    }

    return result;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::parseSensors(const JsonArray& sensors,
                                                                          SensorConfig& config) {
    ValidationResult result;

    if (sensors.size() == 0) {
        addWarning(result, "No sensors configured");
        return result;
    }

    if (sensors.size() > 10) {
        addWarning(result, "More than 10 sensors configured, may impact performance");
    }

    for (JsonVariant v : sensors) {
        JsonObject sensor = v.as<JsonObject>();
        auto result_sensor = validateSensor(sensor);
        if (!result_sensor.valid) {
            result.valid = false;
            for (const auto& err : result_sensor.errors) {
                addError(result, "sensor[" + sensor["id"].as<String>() + "]: " + err);
            }
        }
        for (const auto& warn : result_sensor.warnings) {
            addWarning(result, "sensor[" + sensor["id"].as<String>() + "]: " + warn);
        }
    }

    return result;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::parseActuators(const JsonArray& actuators,
                                                                            ActuatorConfig& config) {
    ValidationResult result;

    if (actuators.size() == 0) {
        addError(result, "At least one actuator (heater) is required");
        return result;
    }

    bool has_heater = false;
    bool has_fan = false;

    for (JsonVariant v : actuators) {
        JsonObject actuator = v.as<JsonObject>();
        auto result_actuator = validateActuator(actuator);
        if (!result_actuator.valid) {
            result.valid = false;
            for (const auto& err : result_actuator.errors) {
                addError(result, "actuator[" + actuator["id"].as<String>() + "]: " + err);
            }
        }
        for (const auto& warn : result_actuator.warnings) {
            addWarning(result, "actuator[" + actuator["id"].as<String>() + "]: " + warn);
        }

        // Track roles
        String role = actuator["role"] | "";
        if (role == "heater") has_heater = true;
        if (role == "fan") has_fan = true;
    }

    if (!has_heater) {
        addError(result, "At least one actuator with role 'heater' is required");
    }

    return result;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::parseDisplay(const JsonObject& display,
                                                                          DisplayConfig& config) {
    ValidationResult result;

    if (!display["enabled"] | false) {
        config.enabled = false;
        return result;
    }

    config.enabled = true;
    config.driver = display["driver"] | "auto";
    config.bus_type = display["bus_type"] | "i2c";
    config.width = display["width"] | 128;
    config.height = display["height"] | 64;
    config.rotation = display["rotation"] | 0;
    config.spi_mosi = display["spi_mosi"] | -1;
    config.spi_sclk = display["spi_sclk"] | -1;
    config.spi_cs = display["spi_cs"] | -1;
    config.dc_pin = display["dc_pin"] | -1;
    config.rst_pin = display["rst_pin"] | -1;
    config.backlight_pin = display["backlight_pin"] | -1;

    if (display.containsKey("fields")) {
        JsonArray fields = display["fields"].as<JsonArray>();
        config.fields.clear();
        for (JsonVariant v : fields) {
            config.fields.push_back(v.as<String>());
        }
    }

    auto display_result = validateDisplay(display);
    if (!display_result.valid) {
        result.valid = false;
        for (const auto& err : display_result.errors) {
            addError(result, err);
        }
    }
    for (const auto& warn : display_result.warnings) {
        addWarning(result, warn);
    }

    return result;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::parseControl(const JsonObject& control,
                                                                          ControlConfig& config) {
    ValidationResult result;

    config.algorithm = control["algorithm"] | "pid";
    config.auto_tune = control["auto_tune"] | false;

    if (control.containsKey("parameters")) {
        JsonObject params = control["parameters"].as<JsonObject>();
        // Parameters are algorithm-specific, stored as raw JSON
    }

    if (control.containsKey("safety_limits")) {
        JsonObject safety = control["safety_limits"].as<JsonObject>();
        config.safety_limits.hard_temp_limit_c = safety["hard_temp_limit_c"] | 80.0f;
        config.safety_limits.max_heater_power_pct = safety["max_heater_power_pct"] | 100;
        config.safety_limits.sensor_timeout_ms = safety["sensor_timeout_ms"] | 600;
        config.safety_limits.thermal_runaway_time_sec = safety["thermal_runaway_time_sec"] | 45;
        config.safety_limits.thermal_runaway_temp_rise_c = safety["thermal_runaway_temp_rise_c"] | 0.5f;
    }

    auto control_result = validateControl(control);
    if (!control_result.valid) {
        result.valid = false;
        for (const auto& err : control_result.errors) {
            addError(result, err);
        }
    }
    for (const auto& warn : control_result.warnings) {
        addWarning(result, warn);
    }

    return result;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::validateSensor(const JsonObject& sensor) {
    ValidationResult result;

    // Required fields
    if (!sensor.containsKey("id") || sensor["id"].isNull()) {
        addError(result, "Sensor missing required 'id' field");
    } else {
        String id = sensor["id"].as<String>();
        if (id.length() == 0) {
            addError(result, "Sensor 'id' cannot be empty");
        }
    }

    if (!sensor.containsKey("type") || sensor["type"].isNull()) {
        addError(result, "Sensor missing required 'type' field");
    } else {
        String type = sensor["type"].as<String>();
        if (!isValidSensorType(type)) {
            addError(result, "Unknown sensor type: " + type);
        }
    }

    if (!sensor.containsKey("capabilities") || !sensor["capabilities"].is<JsonArray>()) {
        addError(result, "Sensor missing required 'capabilities' array");
    } else {
        JsonArray caps = sensor["capabilities"].as<JsonArray>();
        if (caps.size() == 0) {
            addError(result, "Sensor must have at least one capability");
        }
        for (JsonVariant v : caps) {
            String cap = v.as<String>();
            if (cap != "temperature" && cap != "humidity" && cap != "pressure") {
                addError(result, "Unknown capability: " + cap);
            }
        }
    }

    if (!sensor.containsKey("bus") || !sensor["bus"].is<JsonObject>()) {
        addError(result, "Sensor missing required 'bus' configuration object");
    } else {
        JsonObject bus = sensor["bus"].as<JsonObject>();
        String bus_type = bus["type"] | "";
        if (!isValidBusType(bus_type)) {
            addError(result, "Invalid bus type: " + bus_type);
        }

        // Validate bus-specific pins
        if (bus_type == "i2c") {
            int sda = bus["sda_pin"] | 21;
            int scl = bus["scl_pin"] | 22;
            if (!isValidGPIOPin(sda)) addError(result, "Invalid I2C SDA pin: " + String(sda));
            if (!isValidGPIOPin(scl)) addError(result, "Invalid I2C SCL pin: " + String(scl));
        } else if (bus_type == "spi") {
            int mosi = bus["mosi_pin"] | -1;
            int sclk = bus["sclk_pin"] | -1;
            int cs = bus["cs_pin"] | -1;
            if (mosi >= 0 && !isValidGPIOPin(mosi)) addError(result, "Invalid SPI MOSI pin: " + String(mosi));
            if (sclk >= 0 && !isValidGPIOPin(sclk)) addError(result, "Invalid SPI SCLK pin: " + String(sclk));
            if (cs >= 0 && !isValidGPIOPin(cs)) addError(result, "Invalid SPI CS pin: " + String(cs));
        } else if (bus_type == "onewire" || bus_type == "uart" || bus_type == "adc") {
            int pin = bus["pin"] | -1;
            if (pin >= 0 && !isValidGPIOPin(pin)) addError(result, "Invalid GPIO pin for " + bus_type + ": " + String(pin));
        }
    }

    // Driver params (optional)
    if (sensor.containsKey("driver_params")) {
        if (!sensor["driver_params"].is<JsonObject>()) {
            addError(result, "driver_params must be a JSON object");
        }
    }

    // Calibration (optional)
    if (sensor.containsKey("calibration")) {
        JsonObject cal = sensor["calibration"].as<JsonObject>();
        // Validate calibration values are numbers
    }

    return result;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::validateActuator(const JsonObject& actuator) {
    ValidationResult result;

    // Required fields
    if (!actuator.containsKey("id") || actuator["id"].isNull()) {
        addError(result, "Actuator missing required 'id' field");
    } else {
        String id = actuator["id"].as<String>();
        if (id.length() == 0) {
            addError(result, "Actuator 'id' cannot be empty");
        }
    }

    if (!actuator.containsKey("type") || actuator["type"].isNull()) {
        addError(result, "Actuator missing required 'type' field");
    } else {
        String type = actuator["type"].as<String>();
        if (!isValidActuatorType(type)) {
            addError(result, "Unknown actuator type: " + type);
        }
    }

    if (!actuator.containsKey("role") || actuator["role"].isNull()) {
        addError(result, "Actuator missing required 'role' field");
    } else {
        String role = actuator["role"].as<String>();
        if (!isValidActuatorRole(role)) {
            addError(result, "Unknown actuator role: " + role);
        }
    }

    if (!actuator.containsKey("pins") || !actuator["pins"].is<JsonObject>()) {
        addError(result, "Actuator missing required 'pins' object");
    } else {
        JsonObject pins = actuator["pins"].as<JsonObject>();
        if (!pins.containsKey("pwm") || pins["pwm"].isNull()) {
            addError(result, "Actuator pins missing required 'pwm' pin");
        } else {
            int pwm_pin = pins["pwm"].as<int>();
            if (!isValidGPIOPin(pwm_pin)) {
                addError(result, "Invalid PWM pin: " + String(pwm_pin));
            }
        }
    }

    if (!actuator.containsKey("control") || !actuator["control"].is<JsonObject>()) {
        addError(result, "Actuator missing required 'control' configuration object");
    } else {
        JsonObject control = actuator["control"].as<JsonObject>();
        String algorithm = control["algorithm"] | "pid";
        if (!isValidControlAlgorithm(algorithm)) {
            addError(result, "Unknown control algorithm: " + algorithm);
        }

        // Validate algorithm-specific parameters
        if (control.containsKey("parameters")) {
            JsonObject params = control["parameters"].as<JsonObject>();
            if (algorithm == "pid") {
                if (!params.containsKey("kp")) addWarning(result, "PID missing kp parameter");
                if (!params.containsKey("ki")) addWarning(result, "PID missing ki parameter");
                if (!params.containsKey("kd")) addWarning(result, "PID missing kd parameter");
            } else if (algorithm == "bang_bang") {
                if (!params.containsKey("hysteresis_c")) addWarning(result, "Bang-bang missing hysteresis_c parameter");
            } else if (algorithm == "pwm_feedforward") {
                if (!params.containsKey("base_pwm_pct")) addWarning(result, "Feedforward missing base_pwm_pct");
                if (!params.containsKey("temp_coefficient")) addWarning(result, "Feedforward missing temp_coefficient");
            }
        }
    }

    // Safety limits (optional)
    if (actuator.containsKey("safety_limits")) {
        JsonObject safety = actuator["safety_limits"].as<JsonObject>();
        if (safety.containsKey("max_temp_c")) {
            float max_temp = safety["max_temp_c"].as<float>();
            if (max_temp <= 0 || max_temp > 150) {
                addWarning(result, "Safety max_temp_c out of reasonable range: " + String(max_temp));
            }
        }
    }

    return result;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::validateDisplay(const JsonObject& display) {
    ValidationResult result;

    if (display.containsKey("enabled") && !(display["enabled"].as<bool>())) {
        return result; // Disabled, no further validation needed
    }

    String driver = display["driver"] | "auto";
    if (!isValidDisplayDriver(driver)) {
        addError(result, "Unknown display driver: " + driver);
    }

    String bus_type = display["bus_type"] | "i2c";
    if (!isValidBusType(bus_type)) {
        addError(result, "Invalid display bus type: " + bus_type);
    }

    int width = display["width"] | 128;
    int height = display["height"] | 64;
    if (width <= 0 || width > 800 || height <= 0 || height > 480) {
        addWarning(result, "Unusual display resolution: " + String(width) + "x" + String(height));
    }

    int rotation = display["rotation"] | 0;
    if (rotation != 0 && rotation != 90 && rotation != 180 && rotation != 270) {
        addError(result, "Invalid display rotation: " + String(rotation) + " (must be 0, 90, 180, or 270)");
    }

    // Validate bus pins based on bus type
    if (bus_type == "spi") {
        int pins[] = {display["spi_mosi"] | -1, display["spi_sclk"] | -1, 
                      display["spi_cs"] | -1, display["dc_pin"] | -1, 
                      display["rst_pin"] | -1, display["backlight_pin"] | -1};
        const char* names[] = {"MOSI", "SCLK", "CS", "DC", "RST", "BL"};
        for (int i = 0; i < 6; i++) {
            if (pins[i] >= 0 && !isValidGPIOPin(pins[i])) {
                addError(result, "Invalid SPI " + String(names[i]) + " pin: " + String(pins[i]));
            }
        }
    } else if (bus_type == "i2c") {
        int sda = display["sda_pin"] | 21;
        int scl = display["scl_pin"] | 22;
        if (!isValidGPIOPin(sda)) addError(result, "Invalid I2C SDA pin: " + String(sda));
        if (!isValidGPIOPin(scl)) addError(result, "Invalid I2C SCL pin: " + String(scl));
    }

    return result;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::validateControl(const JsonObject& control) {
    ValidationResult result;

    String algorithm = control["algorithm"] | "pid";
    if (!isValidControlAlgorithm(algorithm)) {
        addError(result, "Unknown control algorithm: " + algorithm);
    }

    if (control.containsKey("safety_limits")) {
        JsonObject safety = control["safety_limits"].as<JsonObject>();
        float hard_limit = safety["hard_temp_limit_c"] | 80.0f;
        if (hard_limit < 30 || hard_limit > 150) {
            addWarning(result, "Hard temperature limit out of reasonable range: " + String(hard_limit));
        }

        int max_power = safety["max_heater_power_pct"] | 100;
        if (max_power < 10 || max_power > 100) {
            addWarning(result, "Max heater power percentage out of range: " + String(max_power));
        }
    }

    return result;
}

bool HardwareConfigParser::isValidSensorType(const String& type) const {
    for (size_t i = 0; i < sizeof(SENSOR_TYPES)/sizeof(SENSOR_TYPES[0]); i++) {
        if (type == SENSOR_TYPES[i]) return true;
    }
    return false;
}

bool HardwareConfigParser::isValidActuatorType(const String& type) const {
    for (size_t i = 0; i < sizeof(ACTUATOR_TYPES)/sizeof(ACTUATOR_TYPES[0]); i++) {
        if (type == ACTUATOR_TYPES[i]) return true;
    }
    return false;
}

bool HardwareConfigParser::isValidActuatorRole(const String& role) const {
    for (size_t i = 0; i < sizeof(ACTUATOR_ROLES)/sizeof(ACTUATOR_ROLES[0]); i++) {
        if (role == ACTUATOR_ROLES[i]) return true;
    }
    return false;
}

bool HardwareConfigParser::isValidControlAlgorithm(const String& algorithm) const {
    for (size_t i = 0; i < sizeof(CONTROL_ALGORITHMS)/sizeof(CONTROL_ALGORITHMS[0]); i++) {
        if (algorithm == CONTROL_ALGORITHMS[i]) return true;
    }
    return false;
}

bool HardwareConfigParser::isValidDisplayDriver(const String& driver) const {
    for (size_t i = 0; i < sizeof(DISPLAY_DRIVERS)/sizeof(DISPLAY_DRIVERS[0]); i++) {
        if (driver == DISPLAY_DRIVERS[i]) return true;
    }
    return false;
}

bool HardwareConfigParser::isValidBusType(const String& type) const {
    for (size_t i = 0; i < sizeof(BUS_TYPES)/sizeof(BUS_TYPES[0]); i++) {
        if (type == BUS_TYPES[i]) return true;
    }
    return false;
}

bool HardwareConfigParser::isValidGPIOPin(int pin) const {
    // ESP32 valid GPIO pins: 0-39 (some are input-only: 34-39)
    return pin >= 0 && pin <= 39;
}

bool HardwareConfigParser::isValidI2CAddress(int address) const {
    return address >= 0x08 && address <= 0x77;
}

HardwareConfigParser::ValidationResult HardwareConfigParser::checkPinConflicts(const SensorConfig& sensor,
                                                                               const ActuatorConfig& actuator,
                                                                               const DisplayConfig& display) {
    ValidationResult result;
    std::map<int, String> pin_usage;

    // Check sensor pins
    // Note: This is a simplified version - full implementation would check all pins
    // from sensor.bus, actuator.pins, display pins

    return result;
}

void HardwareConfigParser::addError(ValidationResult& result, const String& message) {
    result.valid = false;
    result.errors.push_back(message);
}

void HardwareConfigParser::addWarning(ValidationResult& result, const String& message) {
    result.warnings.push_back(message);
}

bool HardwareConfigParser::isTypeInArray(const String& type, const char* arr[], size_t count) const {
    for (size_t i = 0; i < count; i++) {
        if (type == arr[i]) return true;
    }
    return false;
}

constexpr const char* HardwareConfigParser::SENSOR_TYPES[];
constexpr const char* HardwareConfigParser::ACTUATOR_TYPES[];
constexpr const char* HardwareConfigParser::ACTUATOR_ROLES[];
constexpr const char* HardwareConfigParser::CONTROL_ALGORITHMS[];
constexpr const char* HardwareConfigParser::DISPLAY_DRIVERS[];
constexpr const char* HardwareConfigParser::BUS_TYPES[];

// Convenience function
HardwareConfigParser::ValidationResult parseHardwareConfig(const String& json_string,
                                                           SensorConfig& sensor,
                                                           ActuatorConfig& actuator,
                                                           DisplayConfig& display,
                                                           ControlConfig& control) {
    HardwareConfigParser parser;
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json_string.c_str());
    if (err) {
        HardwareConfigParser::ValidationResult result;
        result.valid = false;
        result.errors.push_back("JSON parse error: " + String(err.c_str()));
        return result;
    }
    return parser.parse(doc.as<JsonObject>(), sensor, actuator, display, control);
}

} // namespace filament_dryer