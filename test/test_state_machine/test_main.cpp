#include <unity.h>
#include "Arduino.h"
#include "../../src/core/StateMachine.hpp"

using namespace filament_dryer;

void setUp() {
    test_set_millis(0);
}

void tearDown() {}

void test_boot_to_ready_transition() {
    StateMachine sm;
    TEST_ASSERT_TRUE(sm.begin());
    TEST_ASSERT_EQUAL_STRING("BOOT", sm.getStateName().c_str());
    TEST_ASSERT_TRUE(sm.transitionTo(SystemState::WIFI_CONNECT));
    TEST_ASSERT_TRUE(sm.transitionTo(SystemState::READY));
    TEST_ASSERT_EQUAL_STRING("READY", sm.getStateName().c_str());
}

void test_start_and_stop_drying_session() {
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
    TEST_ASSERT_TRUE(sm.isDrying());
    TEST_ASSERT_EQUAL_STRING("DRYING", sm.getStateName().c_str());

    TEST_ASSERT_TRUE(sm.stopDrying(DryingStopReason::USER_STOPPED));
    TEST_ASSERT_FALSE(sm.isDrying());
    TEST_ASSERT_EQUAL_STRING("STOPPED", sm.getStateName().c_str());
}

void test_invalid_transition_rejected() {
    StateMachine sm;
    sm.begin();
    TEST_ASSERT_FALSE(sm.transitionTo(SystemState::DRYING));
}

void test_drying_progress_updates_elapsed_time() {
    StateMachine sm;
    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);

    DryingSession session;
    session.target_temp_c = 55.0f;
    session.max_duration_min = 10;
    sm.startDrying(session);

    test_advance_millis(5000);
    TEST_ASSERT_TRUE(sm.updateDryingProgress(50.0f, 20.0f, 80.0f, true, 50.0f, true));
    TEST_ASSERT_EQUAL_UINT32(5, sm.getCurrentSession().elapsed_sec);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_boot_to_ready_transition);
    RUN_TEST(test_start_and_stop_drying_session);
    RUN_TEST(test_invalid_transition_rejected);
    RUN_TEST(test_drying_progress_updates_elapsed_time);
    return UNITY_END();
}
