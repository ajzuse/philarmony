#include <unity.h>
#include "Arduino.h"
#include "../../src/core/StateMachine.hpp"
#include "../../src/core/SafetyEngine.hpp"
#include "../../src/core/ConfigManager.hpp"

using namespace filament_dryer;

// Flow-oriented tests that exercise P1 user journeys at the domain layer
// (hotspot → ready → drying → fault), with simulated hardware inputs.

void setUp() {
    test_set_millis(0);
}

void tearDown() {}

void test_flow_hotspot_wifi_setup_to_ready() {
    // US1 flow: BOOT → WIFI_CONNECT/HOTSPOT → READY after credentials accepted
    StateMachine sm;
    TEST_ASSERT_TRUE(sm.begin());
    TEST_ASSERT_TRUE(sm.transitionTo(SystemState::HOTSPOT));
    TEST_ASSERT_EQUAL_STRING("HOTSPOT", sm.getStateName().c_str());

    // Simulated successful STA connect after portal submit
    TEST_ASSERT_TRUE(sm.transitionTo(SystemState::READY));
    TEST_ASSERT_TRUE(sm.isReady());
}

void test_flow_websocket_start_stop_status() {
    // US2 flow: READY → DRYING via control/start semantics, then STOPPED
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

    test_advance_millis(2000);
    TEST_ASSERT_TRUE(sm.updateDryingProgress(48.0f, 20.0f, 60.0f, true, 80.0f, true));
    TEST_ASSERT_EQUAL_UINT32(2, sm.getCurrentSession().elapsed_sec);

    TEST_ASSERT_TRUE(sm.stopDrying(DryingStopReason::USER_STOPPED));
    TEST_ASSERT_EQUAL_STRING("STOPPED", sm.getStateName().c_str());
}

void test_flow_sensor_fault_safe_abort() {
    // US3 flow: drying interrupted by sensor disconnect → fault + safe abort semantics
    StateMachine sm;
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    safety.begin(cfg);

    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);

    DryingSession session;
    session.target_temp_c = 55.0f;
    session.max_duration_min = 60;
    sm.startDrying(session);

    TEST_ASSERT_TRUE(safety.checkSafety(50.0f, 55.0f, 80, true, false));
    TEST_ASSERT_TRUE(safety.checkSafety(50.0f, 55.0f, 80, true, false));
    TEST_ASSERT_FALSE(safety.checkSafety(50.0f, 55.0f, 80, true, false));
    TEST_ASSERT_TRUE(safety.isFaulted());
    TEST_ASSERT_EQUAL((int)FaultCode::SENSOR_DISCONNECT, (int)safety.getLastFault());

    // Fault path leaves drying directly into FAULT_STOPPED (safe abort)
    TEST_ASSERT_TRUE(sm.transitionTo(SystemState::FAULT_STOPPED));
    TEST_ASSERT_TRUE(sm.isInFaultState());
}

void test_flow_calibration_and_overtemp_cutoff() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    cfg.hard_temp_limit_c = 80.0f;
    safety.begin(cfg);

    // Simulated calibrated reading still triggers hard limit
    const float calibrated = 40.0f * 1.0f + 42.0f;  // offset applied → 82C
    TEST_ASSERT_FALSE(safety.checkSafety(calibrated, 50.0f, 0, false, true));
    TEST_ASSERT_EQUAL((int)FaultCode::OVER_TEMPERATURE, (int)safety.getLastFault());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_flow_hotspot_wifi_setup_to_ready);
    RUN_TEST(test_flow_websocket_start_stop_status);
    RUN_TEST(test_flow_sensor_fault_safe_abort);
    RUN_TEST(test_flow_calibration_and_overtemp_cutoff);
    return UNITY_END();
}
