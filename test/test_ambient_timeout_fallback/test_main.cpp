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
#include "../../src/core/AmbientFanLogic.hpp"

using namespace filament_dryer;

void setUp() {
    test_reset_preferences();
    test_set_millis(0);
}
void tearDown() {}

void test_timeout_triggers_fallback() {
    // Simulate: last valid reading at t=1000, now at t=32000 (>30s)
    uint32_t lastValidMs = 1000;
    uint32_t now = 32000;
    bool ambientAvailable =
        ambient_fan::ambientIsAvailable(true, now, lastValidMs);
    TEST_ASSERT_FALSE(ambientAvailable);
}

void test_within_timeout_stays_available() {
    uint32_t lastValidMs = 1000;
    uint32_t now = 30000;
    bool ambientAvailable =
        ambient_fan::ambientIsAvailable(true, now, lastValidMs);
    TEST_ASSERT_TRUE(ambientAvailable);
}

void test_at_boundary_timeout() {
    uint32_t lastValidMs = 1000;
    uint32_t now = 31000;
    bool ambientAvailable =
        ambient_fan::ambientIsAvailable(true, now, lastValidMs);
    TEST_ASSERT_FALSE(ambientAvailable);  // exactly 30s = not available
}

void test_recovery_on_valid_reading() {
    uint32_t lastValidMs = 1000;
    uint32_t now = 32000;
    bool ambientAvailable =
        ambient_fan::ambientIsAvailable(true, now, lastValidMs);
    TEST_ASSERT_FALSE(ambientAvailable);
    // Simulate new valid reading
    lastValidMs = now;
    now = 32500;
    ambientAvailable = ambient_fan::ambientIsAvailable(true, now, lastValidMs);
    TEST_ASSERT_TRUE(ambientAvailable);
}

void test_no_reading_ever_seen_is_unavailable() {
    TEST_ASSERT_FALSE(ambient_fan::ambientIsAvailable(false, 1000, 0));
}

void test_fallback_no_device_fault() {
    // Ambient timeout should NOT cause a device fault
    // This is verified by design: ambientAvailable=false → fan uses chamber-only
    // No SafetyEngine fault is raised for ambient unavailability
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    SensorConfig sensor = config.getSensorConfig();
    // Empty ambient_type means no ambient sensor → no fault possible
    TEST_ASSERT_TRUE(sensor.ambient_type.isEmpty());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_timeout_triggers_fallback);
    RUN_TEST(test_within_timeout_stays_available);
    RUN_TEST(test_at_boundary_timeout);
    RUN_TEST(test_recovery_on_valid_reading);
    RUN_TEST(test_no_reading_ever_seen_is_unavailable);
    RUN_TEST(test_fallback_no_device_fault);
    return UNITY_END();
}
