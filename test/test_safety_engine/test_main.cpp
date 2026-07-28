#include <unity.h>
#include "Arduino.h"
#include "../../src/core/SafetyEngine.hpp"

using namespace filament_dryer;

void setUp() {
    test_set_millis(0);
}

void tearDown() {}

void test_hard_temperature_limit_triggers_fault() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.hard_temp_limit_c = 80.0f;
    cfg.watchdog_enabled = false;
    TEST_ASSERT_TRUE(safety.begin(cfg));

    TEST_ASSERT_FALSE(safety.checkSafety(81.0f, 50.0f, 0, false, true));
    TEST_ASSERT_TRUE(safety.isFaulted());
    TEST_ASSERT_EQUAL((int)FaultCode::OVER_TEMPERATURE, (int)safety.getLastFault());
}

void test_sensor_disconnect_after_retries_triggers_fault() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    cfg.sensor_timeout_ms = 600;
    TEST_ASSERT_TRUE(safety.begin(cfg));

    TEST_ASSERT_TRUE(safety.checkSafety(40.0f, 50.0f, 0, false, false));
    test_advance_millis(300);
    TEST_ASSERT_TRUE(safety.checkSafety(40.0f, 50.0f, 0, false, false));
    test_advance_millis(400); // total disconnect duration >= 600ms
    TEST_ASSERT_FALSE(safety.checkSafety(40.0f, 50.0f, 0, false, false));
    TEST_ASSERT_TRUE(safety.isFaulted());
    TEST_ASSERT_EQUAL((int)FaultCode::SENSOR_DISCONNECT, (int)safety.getLastFault());
}

void test_clear_fault_resets_state() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    safety.begin(cfg);
    safety.emergencyStop(FaultCode::OVER_TEMPERATURE, "test");
    safety.clearFault();
    TEST_ASSERT_FALSE(safety.isFaulted());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_hard_temperature_limit_triggers_fault);
    RUN_TEST(test_sensor_disconnect_after_retries_triggers_fault);
    RUN_TEST(test_clear_fault_resets_state);
    return UNITY_END();
}
