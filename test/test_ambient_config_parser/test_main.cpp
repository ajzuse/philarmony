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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <unity.h>
#include <ArduinoJson.h>
#include "../../src/core/ConfigManager.hpp"
#include "../../src/core/HardwareConfigParser.hpp"

using namespace filament_dryer;

void setUp() { test_reset_preferences(); }
void tearDown() {}

void test_ambient_role_parsed_correctly() {
    const char* json = R"({
      "sensors": [
        {"id": "chamber", "type": "sht31", "capabilities": ["temperature", "humidity"],
         "bus": {"type": "i2c", "sda_pin": 21, "scl_pin": 22}},
        {"id": "ambient", "type": "dht22", "role": "ambient",
         "capabilities": ["temperature", "humidity"],
         "bus": {"type": "onewire", "pin": 4}}
      ]
    })";
    JsonDocument doc;
    deserializeJson(doc, json);
    SensorConfig config;
    HardwareConfigParser parser;
    auto result = parser.parseSensors(doc["sensors"].as<JsonArray>(), config);
    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_EQUAL_STRING("sht31", config.type.c_str());
    TEST_ASSERT_EQUAL_STRING("dht22", config.ambient_type.c_str());
}

void test_no_role_defaults_to_chamber() {
    const char* json = R"({
      "sensors": [
        {"id": "chamber", "type": "sht31", "capabilities": ["temperature", "humidity"],
         "bus": {"type": "i2c", "sda_pin": 21, "scl_pin": 22}},
        {"id": "extra", "type": "dht22", "capabilities": ["temperature", "humidity"],
         "bus": {"type": "onewire", "pin": 4}}
      ]
    })";
    JsonDocument doc;
    deserializeJson(doc, json);
    SensorConfig config;
    HardwareConfigParser parser;
    auto result = parser.parseSensors(doc["sensors"].as<JsonArray>(), config);
    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_TRUE(config.ambient_type.isEmpty());
}

void test_ambient_without_humidity_ignored() {
    const char* json = R"({
      "sensors": [
        {"id": "chamber", "type": "sht31", "capabilities": ["temperature", "humidity"],
         "bus": {"type": "i2c", "sda_pin": 21, "scl_pin": 22}},
        {"id": "ambient", "type": "ds18b20", "role": "ambient",
         "capabilities": ["temperature"],
         "bus": {"type": "onewire", "pin": 4}}
      ]
    })";
    JsonDocument doc;
    deserializeJson(doc, json);
    SensorConfig config;
    HardwareConfigParser parser;
    auto result = parser.parseSensors(doc["sensors"].as<JsonArray>(), config);
    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_TRUE(config.ambient_type.isEmpty());
}

void test_ambient_different_model() {
    const char* json = R"({
      "sensors": [
        {"id": "chamber", "type": "bme280", "capabilities": ["temperature", "humidity"],
         "bus": {"type": "i2c", "sda_pin": 21, "scl_pin": 22}},
        {"id": "ambient", "type": "aht20", "role": "ambient",
         "capabilities": ["temperature", "humidity"],
         "bus": {"type": "i2c", "sda_pin": 16, "scl_pin": 17, "address": 56}}
      ]
    })";
    JsonDocument doc;
    deserializeJson(doc, json);
    SensorConfig config;
    HardwareConfigParser parser;
    auto result = parser.parseSensors(doc["sensors"].as<JsonArray>(), config);
    for (const auto& e : result.errors) printf("ERR: %s\n", e.c_str());
    printf("ambient='%s' extra='%s' type='%s'\n",
           config.ambient_type.c_str(), config.extra_temp_type.c_str(), config.type.c_str());
    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_EQUAL_STRING("bme280", config.type.c_str());
    TEST_ASSERT_EQUAL_STRING("aht20", config.ambient_type.c_str());
}

void test_ambient_pin_conflict_rejected() {
    const char* json = R"({
      "sensors": [
        {"id": "chamber", "type": "dht22", "capabilities": ["temperature", "humidity"],
         "bus": {"type": "onewire", "pin": 4}},
        {"id": "ambient", "type": "dht22", "role": "ambient",
         "capabilities": ["temperature", "humidity"],
         "bus": {"type": "onewire", "pin": 4}}
      ]
    })";
    JsonDocument doc;
    deserializeJson(doc, json);
    SensorConfig sensor;
    ActuatorConfig actuator;
    DisplayConfig display;
    ControlConfig control;
    HardwareConfigParser parser;
    auto result = parser.parse(doc.as<JsonObject>(), sensor, actuator, display, control);
    TEST_ASSERT_FALSE(result.valid);
    bool conflict_reported = false;
    for (const auto& e : result.errors) {
        if (e.indexOf("conflict") >= 0) conflict_reported = true;
    }
    TEST_ASSERT_TRUE(conflict_reported);
}

void test_ambient_bus_does_not_override_chamber_bus() {
    const char* json = R"({
      "sensors": [
        {"id": "chamber", "type": "sht31", "capabilities": ["temperature", "humidity"],
         "bus": {"type": "i2c", "sda_pin": 21, "scl_pin": 22, "bus": 1}},
        {"id": "ambient", "type": "dht22", "role": "ambient",
         "capabilities": ["temperature", "humidity"],
         "bus": {"type": "onewire", "pin": 4, "bus": 2}}
      ]
    })";
    JsonDocument doc;
    deserializeJson(doc, json);
    SensorConfig config;
    HardwareConfigParser parser;
    auto result = parser.parseSensors(doc["sensors"].as<JsonArray>(), config);
    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_EQUAL_STRING("dht22", config.ambient_type.c_str());
    // Ambient bus key must not mutate the chamber i2c_bus.
    TEST_ASSERT_EQUAL_UINT8(1, config.i2c_bus);
}

void test_ambient_unsupported_type_rejected() {
    const char* json = R"({
      "sensors": [
        {"id": "chamber", "type": "sht31", "capabilities": ["temperature", "humidity"],
         "bus": {"type": "i2c", "sda_pin": 21, "scl_pin": 22}},
        {"id": "ambient", "type": "foo42", "role": "ambient",
         "capabilities": ["temperature", "humidity"],
         "bus": {"type": "i2c", "sda_pin": 16, "scl_pin": 17}}
      ]
    })";
    JsonDocument doc;
    deserializeJson(doc, json);
    SensorConfig sensor;
    ActuatorConfig actuator;
    DisplayConfig display;
    ControlConfig control;
    HardwareConfigParser parser;
    auto result = parser.parse(doc.as<JsonObject>(), sensor, actuator, display, control);
    TEST_ASSERT_FALSE(result.valid);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_ambient_role_parsed_correctly);
    RUN_TEST(test_no_role_defaults_to_chamber);
    RUN_TEST(test_ambient_without_humidity_ignored);
    RUN_TEST(test_ambient_different_model);
    RUN_TEST(test_ambient_pin_conflict_rejected);
    RUN_TEST(test_ambient_bus_does_not_override_chamber_bus);
    RUN_TEST(test_ambient_unsupported_type_rejected);
    return UNITY_END();
}
