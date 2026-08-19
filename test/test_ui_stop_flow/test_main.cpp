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

void test_controller_stop_from_drying_and_paused() {
    ConfigManager config;
    config.begin();
    ProfileManager profiles(config);

    StateMachine drying_state;
    drying_state.begin();
    drying_state.transitionTo(SystemState::WIFI_CONNECT);
    drying_state.transitionTo(SystemState::READY);
    TouchUiController drying(drying_state, profiles, config);
    drying.startFromProfile("pla");
    TEST_ASSERT_TRUE(drying.stop());
    TEST_ASSERT_EQUAL(DryingStopReason::USER_STOPPED,
                      drying_state.getCurrentSession().stop_reason);

    StateMachine paused_state;
    paused_state.begin();
    paused_state.transitionTo(SystemState::WIFI_CONNECT);
    paused_state.transitionTo(SystemState::READY);
    TouchUiController paused(paused_state, profiles, config);
    paused.startFromProfile("petg");
    paused.pause();
    TEST_ASSERT_TRUE(paused.stop());
    TEST_ASSERT_EQUAL(SystemState::STOPPED, paused_state.getState());
    TEST_ASSERT_FALSE(paused_state.getCurrentSession().heater_on);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_controller_stop_from_drying_and_paused);
    return UNITY_END();
}
