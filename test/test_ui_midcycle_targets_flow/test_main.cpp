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
#include "../../src/core/ProfileManager.hpp"
#include "../../src/core/StateMachine.hpp"
#include "../../src/ui/TouchUiController.hpp"

using namespace filament_dryer;

void setUp() {
    test_reset_preferences();
    test_set_millis(0);
}
void tearDown() {}

void test_midcycle_apply_updates_all_targets_and_validates_ranges() {
    ConfigManager config;
    config.begin();
    ProfileManager profiles(config);
    StateMachine state;
    state.begin();
    state.transitionTo(SystemState::WIFI_CONNECT);
    state.transitionTo(SystemState::READY);
    TouchUiController controller(state, profiles, config);
    controller.startFromProfile("pla");

    TEST_ASSERT_FALSE(controller.applyTargets(90.0f, 12.0f, 180));
    TEST_ASSERT_FALSE(controller.applyTargets(55.0f, 2.0f, 180));
    TEST_ASSERT_FALSE(controller.applyTargets(55.0f, 12.0f, 0));
    TEST_ASSERT_TRUE(controller.applyTargets(55.0f, 12.0f, 180));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 55.0f,
                             state.getCurrentSession().target_temp_c);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.0f,
                             state.getCurrentSession().target_humidity_pct);
    TEST_ASSERT_EQUAL_UINT16(180,
                             state.getCurrentSession().max_duration_min);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_midcycle_apply_updates_all_targets_and_validates_ranges);
    return UNITY_END();
}
