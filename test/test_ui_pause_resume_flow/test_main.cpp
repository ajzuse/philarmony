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

void test_controller_pause_resume_flow_freezes_elapsed() {
    ConfigManager config;
    config.begin();
    ProfileManager profiles(config);
    StateMachine state;
    state.begin();
    state.transitionTo(SystemState::WIFI_CONNECT);
    state.transitionTo(SystemState::READY);
    TouchUiController controller(state, profiles, config);
    TEST_ASSERT_TRUE(controller.startFromProfile("pla"));

    test_advance_millis(10000);
    state.updateDryingProgress(45.0f, 20.0f, 50.0f, true, 40.0f, true);
    TEST_ASSERT_TRUE(controller.pause());
    TEST_ASSERT_TRUE(state.isPaused());
    TEST_ASSERT_FALSE(state.getCurrentSession().heater_on);
    const uint32_t elapsed = state.getSessionUptime();
    test_advance_millis(5000);
    TEST_ASSERT_EQUAL_UINT32(elapsed, state.getSessionUptime());

    TEST_ASSERT_TRUE(controller.resume());
    TEST_ASSERT_TRUE(state.isDrying());
    TEST_ASSERT_EQUAL_STRING("drying", state.getStatusStreamName().c_str());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_controller_pause_resume_flow_freezes_elapsed);
    return UNITY_END();
}
