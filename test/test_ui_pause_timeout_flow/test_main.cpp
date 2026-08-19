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

void test_controller_reports_pause_timeout_once() {
    ConfigManager config;
    config.begin();
    ProfileManager profiles(config);
    StateMachine state;
    state.begin();
    state.transitionTo(SystemState::WIFI_CONNECT);
    state.transitionTo(SystemState::READY);
    TouchUiController controller(state, profiles, config);
    TEST_ASSERT_TRUE(controller.startFromProfile("pla"));
    TEST_ASSERT_TRUE(controller.pause());

    test_advance_millis(StateMachine::kPauseTimeoutMs + 1);
    controller.tick();
    TEST_ASSERT_EQUAL(SystemState::STOPPED, state.getState());
    TEST_ASSERT_EQUAL(DryingStopReason::PAUSE_TIMEOUT,
                      state.getCurrentSession().stop_reason);
    TEST_ASSERT_TRUE(controller.consumePauseTimeoutEvent());
    TEST_ASSERT_FALSE(controller.consumePauseTimeoutEvent());
}

void test_external_state_machine_timeout_is_observed_by_controller() {
    ConfigManager config;
    config.begin();
    ProfileManager profiles(config);
    StateMachine state;
    state.begin();
    state.transitionTo(SystemState::WIFI_CONNECT);
    state.transitionTo(SystemState::READY);
    TouchUiController controller(state, profiles, config);
    controller.startFromProfile("pla");
    controller.pause();

    test_advance_millis(StateMachine::kPauseTimeoutMs + 1);
    TEST_ASSERT_TRUE(state.tickPauseTimeout());
    controller.tick();
    TEST_ASSERT_TRUE(controller.consumePauseTimeoutEvent());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_controller_reports_pause_timeout_once);
    RUN_TEST(test_external_state_machine_timeout_is_observed_by_controller);
    return UNITY_END();
}
