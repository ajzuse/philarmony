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

static void makeReady(StateMachine& state) {
    TEST_ASSERT_TRUE(state.begin());
    TEST_ASSERT_TRUE(state.transitionTo(SystemState::WIFI_CONNECT));
    TEST_ASSERT_TRUE(state.transitionTo(SystemState::READY));
}

void test_start_from_profile_uses_profile_manager_defaults() {
    ConfigManager config;
    TEST_ASSERT_TRUE(config.begin());
    ProfileManager profiles(config);
    TEST_ASSERT_GREATER_THAN(0, profiles.listProfiles().size());
    StateMachine state;
    makeReady(state);
    TouchUiController controller(state, profiles, config);

    TEST_ASSERT_TRUE(controller.startFromProfile("pla"));
    TEST_ASSERT_TRUE(state.isDrying());
    TEST_ASSERT_EQUAL_STRING("pla",
                             state.getCurrentSession().profile_id.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f,
                             state.getCurrentSession().target_temp_c);
    TEST_ASSERT_EQUAL_UINT16(240,
                             state.getCurrentSession().max_duration_min);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_start_from_profile_uses_profile_manager_defaults);
    return UNITY_END();
}
