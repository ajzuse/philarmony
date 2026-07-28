#include <unity.h>
#include "Arduino.h"
#include <ArduinoJson.h>
#include "../../src/core/HardwareConfigParser.hpp"

using namespace filament_dryer;

void setUp() {}
void tearDown() {}

void test_valid_sensor_type() {
    HardwareConfigParser parser;
    TEST_ASSERT_TRUE(parser.isValidSensorType("sht3x"));
    TEST_ASSERT_TRUE(parser.isValidSensorType("dht22"));
    TEST_ASSERT_FALSE(parser.isValidSensorType("invalid_sensor"));
}

void test_parse_minimal_hardware_config() {
    const char* json = R"({
      "sensors": [{
        "id": "chamber",
        "type": "sht3x",
        "capabilities": ["temperature", "humidity"],
        "bus": { "type": "i2c", "bus": 0, "address": 68, "sda_pin": 21, "scl_pin": 22 }
      }],
      "actuators": [{
        "id": "heater",
        "type": "mosfet_pwm",
        "role": "heater",
        "pins": { "pwm": 25 },
        "control": { "algorithm": "pid", "pwm_freq_hz": 1000, "max_power_pct": 100 }
      }],
      "display": {
        "enabled": true,
        "driver": "ssd1306",
        "bus": { "type": "i2c", "address": 60, "sda_pin": 21, "scl_pin": 22 },
        "geometry": { "width": 128, "height": 64, "rotation": 0 }
      },
      "control": {
        "algorithm": "pid",
        "parameters": { "kp": 1.0, "ki": 0.1, "kd": 0.01 }
      }
    })";

    JsonDocument doc;
    TEST_ASSERT(deserializeJson(doc, json) == DeserializationError::Ok);

    HardwareConfigParser parser;
    SensorConfig sensor;
    ActuatorConfig actuator;
    DisplayConfig display;
    ControlConfig control;

    auto result = parser.parse(doc.as<JsonObject>(), sensor, actuator, display, control);
    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_EQUAL_STRING("pid", control.algorithm.c_str());
}

void test_invalid_gpio_pin_rejected() {
    HardwareConfigParser parser;
    TEST_ASSERT_FALSE(parser.isValidGPIOPin(99));
    TEST_ASSERT_TRUE(parser.isValidGPIOPin(25));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_valid_sensor_type);
    RUN_TEST(test_parse_minimal_hardware_config);
    RUN_TEST(test_invalid_gpio_pin_rejected);
    return UNITY_END();
}
