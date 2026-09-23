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
#include "Preferences.h"
#include "../../src/core/ConfigManager.hpp"

using namespace filament_dryer;

void setUp() { test_reset_preferences(); }
void tearDown() {}

void test_ambient_fields_persist_and_load() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());

    SensorConfig sensor = config.getSensorConfig();
    sensor.ambient_type = "dht22";
    sensor.ambient_i2c_address = 0x38;
    sensor.ambient_gpio_pin = 4;
    sensor.ambient_sda_pin = 16;
    sensor.ambient_scl_pin = 17;
    sensor.ambient_temperature_offset = 1.5f;
    sensor.ambient_temperature_scale = 0.98f;
    sensor.ambient_humidity_offset = -2.0f;
    sensor.ambient_humidity_scale = 1.02f;
    config.setSensorConfig(sensor);

    SensorConfig loaded = config.getSensorConfig();
    TEST_ASSERT_EQUAL_STRING("dht22", loaded.ambient_type.c_str());
    TEST_ASSERT_EQUAL_UINT8(0x38, loaded.ambient_i2c_address);
    TEST_ASSERT_EQUAL_INT8(4, loaded.ambient_gpio_pin);
    TEST_ASSERT_EQUAL_INT8(16, loaded.ambient_sda_pin);
    TEST_ASSERT_EQUAL_INT8(17, loaded.ambient_scl_pin);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.5f, loaded.ambient_temperature_offset);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.98f, loaded.ambient_temperature_scale);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -2.0f, loaded.ambient_humidity_offset);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.02f, loaded.ambient_humidity_scale);
}

void test_defaults_when_ambient_not_configured() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    SensorConfig sensor = config.getSensorConfig();
    TEST_ASSERT_TRUE(sensor.ambient_type.isEmpty());
    TEST_ASSERT_EQUAL_INT8(-1, sensor.ambient_gpio_pin);
    TEST_ASSERT_EQUAL_UINT8(0, sensor.ambient_i2c_address);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, sensor.ambient_temperature_offset);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, sensor.ambient_temperature_scale);
}

void test_ventilation_margin_persist_and_load() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    UISettings settings = config.getUiSettings();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, settings.ambient_ventilation_margin_pct);
    settings.ambient_ventilation_margin_pct = 7.5f;
    config.setUiSettings(settings);
    UISettings loaded = config.getUiSettings();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 7.5f, loaded.ambient_ventilation_margin_pct);
}

void test_ventilation_margin_clamp() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    UISettings settings = config.getUiSettings();
    settings.ambient_ventilation_margin_pct = 15.0f;  // out of range
    config.setUiSettings(settings);
    UISettings loaded = config.getUiSettings();
    // Out-of-range values are clamped to the supported 0-10 %RH window.
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, loaded.ambient_ventilation_margin_pct);

    settings.ambient_ventilation_margin_pct = -3.0f;
    config.setUiSettings(settings);
    loaded = config.getUiSettings();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, loaded.ambient_ventilation_margin_pct);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_ambient_fields_persist_and_load);
    RUN_TEST(test_defaults_when_ambient_not_configured);
    RUN_TEST(test_ventilation_margin_persist_and_load);
    RUN_TEST(test_ventilation_margin_clamp);
    return UNITY_END();
}
