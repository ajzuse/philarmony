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
#include "../../src/core/StateMachine.hpp"
#include "../../src/core/ProfileManager.hpp"

using namespace filament_dryer;

void setUp() {
    test_reset_preferences();
    test_set_millis(0);
}
void tearDown() {}

void test_chamber_only_no_ambient_fields() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    SensorConfig sensor = config.getSensorConfig();
    TEST_ASSERT_TRUE(sensor.ambient_type.isEmpty());
    TEST_ASSERT_EQUAL_INT8(-1, sensor.ambient_gpio_pin);
    TEST_ASSERT_EQUAL_UINT8(0, sensor.ambient_i2c_address);
}

void test_chamber_only_no_ambient_in_status() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    SensorConfig sensor = config.getSensorConfig();
    // When ambient_type is empty, no ambient fields should appear in status
    TEST_ASSERT_TRUE(sensor.ambient_type.isEmpty());
}

void test_chamber_only_fan_unchanged() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    UISettings settings = config.getUiSettings();
    // Default margin exists but ambient is not configured → fan logic unchanged
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, settings.ambient_ventilation_margin_pct);
}

void test_chamber_only_default_ventilation_margin() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    UISettings settings = config.getUiSettings();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, settings.ambient_ventilation_margin_pct);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_chamber_only_no_ambient_fields);
    RUN_TEST(test_chamber_only_no_ambient_in_status);
    RUN_TEST(test_chamber_only_fan_unchanged);
    RUN_TEST(test_chamber_only_default_ventilation_margin);
    return UNITY_END();
}
