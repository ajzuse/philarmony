#include <unity.h>
#include "Arduino.h"
#include "../../src/core/StateMachine.hpp"

using namespace filament_dryer;

void setUp() { test_set_millis(0); }
void tearDown() {}

void test_pause_resume_freezes_elapsed() {
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

    test_advance_millis(10000);
    sm.updateDryingProgress(45.0f, 20.0f, 50.0f, true, 40.0f, true);
    TEST_ASSERT_EQUAL_UINT32(10, sm.getCurrentSession().elapsed_sec);

    TEST_ASSERT_TRUE(sm.pauseDrying());
    TEST_ASSERT_TRUE(sm.isPaused());
    TEST_ASSERT_EQUAL_STRING("paused", sm.getStatusStreamName().c_str());
    TEST_ASSERT_FALSE(sm.getCurrentSession().heater_on);

    test_advance_millis(5000);
    TEST_ASSERT_EQUAL_UINT32(10, sm.getSessionUptime());

    TEST_ASSERT_TRUE(sm.resumeDrying());
    TEST_ASSERT_TRUE(sm.isDrying());
}

void test_pause_timeout_auto_stop() {
    StateMachine sm;
    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);
    DryingSession session;
    session.max_duration_min = 60;
    session.target_temp_c = 50.0f;
    sm.startDrying(session);
    TEST_ASSERT_TRUE(sm.pauseDrying());

    test_advance_millis(StateMachine::kPauseTimeoutMs - 1000);
    TEST_ASSERT_FALSE(sm.tickPauseTimeout());
    TEST_ASSERT_TRUE(sm.isPaused());

    test_advance_millis(2000);
    TEST_ASSERT_TRUE(sm.tickPauseTimeout());
    TEST_ASSERT_EQUAL_STRING("STOPPED", sm.getStateName().c_str());
    TEST_ASSERT_EQUAL_STRING("pause_timeout",
        StateMachine::stopReasonToString(sm.getCurrentSession().stop_reason).c_str());
}

void test_stop_from_paused() {
    StateMachine sm;
    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);
    DryingSession session;
    session.max_duration_min = 10;
    session.target_temp_c = 50.0f;
    sm.startDrying(session);
    sm.pauseDrying();
    TEST_ASSERT_TRUE(sm.stopDrying(DryingStopReason::USER_STOPPED));
    TEST_ASSERT_EQUAL_STRING("STOPPED", sm.getStateName().c_str());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_pause_resume_freezes_elapsed);
    RUN_TEST(test_pause_timeout_auto_stop);
    RUN_TEST(test_stop_from_paused);
    return UNITY_END();
}
