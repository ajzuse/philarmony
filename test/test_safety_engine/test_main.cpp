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

void test_sensor_rate_of_change_uses_sensor_fault_code() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    cfg.max_temp_rate_c_per_sec = 2.0f;
    safety.begin(cfg);
    TEST_ASSERT_TRUE(safety.checkSafety(40.0f, 50.0f, 0, false, true));
    test_advance_millis(100); // 0.1s → rate would be 100 C/s if jump +10
    TEST_ASSERT_FALSE(safety.checkSafety(50.0f, 50.0f, 0, false, true));
    TEST_ASSERT_EQUAL((int)FaultCode::SENSOR_RATE_OF_CHANGE, (int)safety.getLastFault());
}

void test_actuator_fault_requires_real_overcurrent_flag() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    safety.begin(cfg);
    // Echo commanded==measured without feedback flag must NOT fault
    TEST_ASSERT_TRUE(safety.checkActuatorFault(80.0f, 80.0f, false, false));
    TEST_ASSERT_FALSE(safety.isFaulted());
    // Real overcurrent flag trips
    TEST_ASSERT_FALSE(safety.checkActuatorFault(80.0f, 10.0f, true, true));
    TEST_ASSERT_EQUAL((int)FaultCode::ACTUATOR_FAULT, (int)safety.getLastFault());
}

void test_spi_recovery_callback_avoids_fault_when_ok() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    safety.begin(cfg);
    safety.setSpiRecoveryCallback([]() { return true; });
    TEST_ASSERT_TRUE(safety.detectAndRecoverSpiBusError(true));
    TEST_ASSERT_FALSE(safety.isFaulted());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_hard_temperature_limit_triggers_fault);
    RUN_TEST(test_sensor_disconnect_after_retries_triggers_fault);
    RUN_TEST(test_clear_fault_resets_state);
    RUN_TEST(test_sensor_rate_of_change_uses_sensor_fault_code);
    RUN_TEST(test_actuator_fault_requires_real_overcurrent_flag);
    RUN_TEST(test_spi_recovery_callback_avoids_fault_when_ok);
    return UNITY_END();
}
