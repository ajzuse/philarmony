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
    TEST_ASSERT_TRUE(parser.isValidSensorType("sht31"));
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
    TEST_ASSERT_EQUAL_UINT16(128, display.width);
    TEST_ASSERT_EQUAL_UINT16(64, display.height);
    TEST_ASSERT_EQUAL_STRING("i2c", display.bus_type.c_str());
}

void test_parse_nested_spi_display_bus() {
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
        "driver": "st7789",
        "bus": { "type": "spi", "mosi": 19, "sclk": 18, "cs": 5, "dc": 16, "rst": 23, "bl": 4 },
        "geometry": { "width": 135, "height": 240, "rotation": 90 }
      },
      "control": { "algorithm": "pid", "parameters": { "kp": 1, "ki": 0, "kd": 0 } }
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
    TEST_ASSERT_EQUAL_STRING("spi", display.bus_type.c_str());
    TEST_ASSERT_EQUAL(19, display.spi_mosi);
    TEST_ASSERT_EQUAL(18, display.spi_sclk);
    TEST_ASSERT_EQUAL(5, display.spi_cs);
    TEST_ASSERT_EQUAL_UINT16(135, display.width);
    TEST_ASSERT_EQUAL_UINT16(240, display.height);
}

void test_invalid_gpio_pin_rejected() {
    HardwareConfigParser parser;
    TEST_ASSERT_FALSE(parser.isValidGPIOPin(99));
    TEST_ASSERT_TRUE(parser.isValidGPIOPin(25));
    TEST_ASSERT_TRUE(parser.isValidGPIOPin(34));  // input-only OK for sensors/ADC
    TEST_ASSERT_TRUE(parser.isValidOutputGPIOPin(25));
    TEST_ASSERT_FALSE(parser.isValidOutputGPIOPin(34));  // input-only rejected for outputs
}

void test_reject_custom_algorithm() {
    HardwareConfigParser parser;
    JsonDocument doc;
    doc["algorithm"] = "custom";
    auto result = parser.validateControl(doc.as<JsonObject>());
    TEST_ASSERT_FALSE(result.valid);
    TEST_ASSERT_TRUE(result.errors.size() > 0);
}

void test_reject_triac_and_parallel_8bit() {
    HardwareConfigParser parser;
    JsonDocument act;
    act["id"] = "h";
    act["type"] = "triac";
    act["role"] = "heater";
    act["pins"]["pwm"] = 25;
    act["control"]["algorithm"] = "pid";
    auto act_result = parser.validateActuator(act.as<JsonObject>());
    TEST_ASSERT_FALSE(act_result.valid);

    JsonDocument disp;
    disp["enabled"] = true;
    disp["driver"] = "hd44780";
    disp["bus_type"] = "parallel_8bit";
    auto disp_result = parser.validateDisplay(disp.as<JsonObject>());
    TEST_ASSERT_FALSE(disp_result.valid);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_valid_sensor_type);
    RUN_TEST(test_parse_minimal_hardware_config);
    RUN_TEST(test_parse_nested_spi_display_bus);
    RUN_TEST(test_invalid_gpio_pin_rejected);
    RUN_TEST(test_reject_custom_algorithm);
    RUN_TEST(test_reject_triac_and_parallel_8bit);
    return UNITY_END();
}
