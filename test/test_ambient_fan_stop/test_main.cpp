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
#include "../../src/core/AmbientFanLogic.hpp"

using namespace filament_dryer;

void setUp() {
    test_reset_preferences();
    test_set_millis(0);
}
void tearDown() {}

static bool isAmbientFanStop(float chamberHumidity, float ambientHumidity,
                              float margin) {
    return ambient_fan::shouldStopFanForAmbient(true, chamberHumidity,
                                                ambientHumidity, margin);
}

void test_fan_stops_when_chamber_within_margin_of_ambient() {
    TEST_ASSERT_TRUE(isAmbientFanStop(18.0f, 16.0f, 2.0f));  // 18 <= 16+2
    TEST_ASSERT_TRUE(isAmbientFanStop(16.0f, 16.0f, 2.0f));  // 16 <= 16+2
    TEST_ASSERT_TRUE(isAmbientFanStop(15.0f, 16.0f, 2.0f));  // 15 <= 16+2
}

void test_fan_runs_when_chamber_above_margin() {
    TEST_ASSERT_FALSE(isAmbientFanStop(20.0f, 16.0f, 2.0f));  // 20 > 16+2
    TEST_ASSERT_FALSE(isAmbientFanStop(19.0f, 16.0f, 2.0f));  // 19 > 16+2
}

void test_fan_stops_when_ambient_higher_than_chamber() {
    TEST_ASSERT_TRUE(isAmbientFanStop(30.0f, 55.0f, 2.0f));  // 30 <= 55+2
}

void test_fallback_when_ambient_invalid() {
    // NaN ambient → should NOT trigger fan stop (chamber-only fallback)
    TEST_ASSERT_FALSE(isAmbientFanStop(18.0f, NAN, 2.0f));
    TEST_ASSERT_FALSE(isAmbientFanStop(NAN, 16.0f, 2.0f));
}

void test_margin_zero_strict_equality() {
    TEST_ASSERT_TRUE(isAmbientFanStop(16.0f, 16.0f, 0.0f));   // 16 <= 16+0
    TEST_ASSERT_FALSE(isAmbientFanStop(17.0f, 16.0f, 0.0f));  // 17 > 16+0
}

void test_margin_configurable_range() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    UISettings settings = config.getUiSettings();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, settings.ambient_ventilation_margin_pct);
    settings.ambient_ventilation_margin_pct = 5.0f;
    TEST_ASSERT_TRUE(config.setUiSettings(settings));
    UISettings loaded = config.getUiSettings();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, loaded.ambient_ventilation_margin_pct);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_fan_stops_when_chamber_within_margin_of_ambient);
    RUN_TEST(test_fan_runs_when_chamber_above_margin);
    RUN_TEST(test_fan_stops_when_ambient_higher_than_chamber);
    RUN_TEST(test_fallback_when_ambient_invalid);
    RUN_TEST(test_margin_zero_strict_equality);
    RUN_TEST(test_margin_configurable_range);
    return UNITY_END();
}
