#include <unity.h>
#include "Arduino.h"
#include "../../src/core/StateMachine.hpp"

using namespace filament_dryer;

// Lightweight stand-in for start-from-profile happy path (ProfileManager needs NVS)
void test_start_preset_state_path() {
    StateMachine sm;
    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);
    DryingSession session;
    session.profile_id = "pla";
    session.target_temp_c = 50.0f;
    session.target_humidity_pct = 15.0f;
    session.max_duration_min = 240;
    TEST_ASSERT_TRUE(sm.startDrying(session));
    TEST_ASSERT_EQUAL_STRING("pla", sm.getCurrentSession().profile_id.c_str());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_start_preset_state_path);
    return UNITY_END();
}
