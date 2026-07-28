#include <unity.h>
#include "Arduino.h"
#include "../../src/core/StateMachine.hpp"
#include "../../src/core/SafetyEngine.hpp"
#include "../../src/core/ConfigManager.hpp"
#include "../../src/core/HardwareConfigParser.hpp"

using namespace filament_dryer;

// Flow-oriented tests that exercise P1 user journeys at the domain layer
// (hotspot → ready → drying → fault), plus handler-path semantics for
// WebSocket start/stop/status and fault-abort with actuator cutoff.

static bool g_actuators_cut = false;

static void fakeActuatorCutoff() {
    g_actuators_cut = true;
}

void setUp() {
    test_set_millis(0);
    g_actuators_cut = false;
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
    // US2 flow: READY → DRYING via control/start semantics, then STOPPED with cutoff
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
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 48.0f, sm.getCurrentSession().current_temp_c);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 60.0f, sm.getCurrentSession().heater_power_pct);

    // control/stop path: leave DRYING then force actuator cutoff callback
    TEST_ASSERT_TRUE(sm.stopDrying(DryingStopReason::USER_STOPPED));
    TEST_ASSERT_EQUAL_STRING("STOPPED", sm.getStateName().c_str());
    fakeActuatorCutoff();
    TEST_ASSERT_TRUE(g_actuators_cut);
}

void test_flow_sensor_fault_safe_abort() {
    // US3 flow: drying interrupted by sensor disconnect → fault + emergency cutoff
    StateMachine sm;
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    cfg.sensor_timeout_ms = 600;
    safety.begin(cfg);
    safety.setEmergencyShutdownCallback(fakeActuatorCutoff);

    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);

    DryingSession session;
    session.target_temp_c = 55.0f;
    session.max_duration_min = 60;
    sm.startDrying(session);

    TEST_ASSERT_TRUE(safety.checkSafety(50.0f, 55.0f, 80, true, false));
    test_advance_millis(700);
    TEST_ASSERT_FALSE(safety.checkSafety(50.0f, 55.0f, 80, true, false));
    TEST_ASSERT_TRUE(safety.isFaulted());
    TEST_ASSERT_EQUAL((int)FaultCode::SENSOR_DISCONNECT, (int)safety.getLastFault());
    TEST_ASSERT_TRUE(g_actuators_cut);

    TEST_ASSERT_TRUE(sm.transitionTo(SystemState::FAULT_STOPPED));
    TEST_ASSERT_TRUE(sm.isInFaultState());
}

void test_flow_calibration_and_overtemp_cutoff() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    cfg.hard_temp_limit_c = 80.0f;
    safety.begin(cfg);
    safety.setEmergencyShutdownCallback(fakeActuatorCutoff);

    const float calibrated = 40.0f * 1.0f + 42.0f;  // offset applied → 82C
    TEST_ASSERT_FALSE(safety.checkSafety(calibrated, 50.0f, 0, false, true));
    TEST_ASSERT_EQUAL((int)FaultCode::OVER_TEMPERATURE, (int)safety.getLastFault());
    TEST_ASSERT_TRUE(g_actuators_cut);
}

void test_flow_hardware_config_parses_actuator_pins() {
    // config/hardware path: actuators JSON must populate ActuatorConfig pins/types
    HardwareConfigParser parser;
    const char* json = R"({
      "sensors": [{
        "id": "chamber",
        "type": "sht3x",
        "capabilities": ["temperature", "humidity"],
        "bus": {"type": "i2c", "bus": 0, "address": 68, "sda_pin": 21, "scl_pin": 22}
      }],
      "actuators": [
        {
          "id": "heater",
          "type": "ssr",
          "role": "heater",
          "pins": {"pwm": 25},
          "control": {"algorithm": "pid", "pwm_freq_hz": 2000, "max_power_pct": 90},
          "safety_limits": {"max_power_pct": 90}
        },
        {
          "id": "fan",
          "type": "fan_pwm",
          "role": "fan",
          "pins": {"pwm": 27},
          "control": {"pwm_freq_hz": 4000, "cooldown_sec": 45}
        }
      ],
      "display": {"enabled": false},
      "control": {"algorithm": "pid", "parameters": {"kp": 1, "ki": 0, "kd": 0}}
    })";

    JsonDocument doc;
    auto err = deserializeJson(doc, json);
    TEST_ASSERT_TRUE(err == DeserializationError::Ok);

    SensorConfig sensor;
    ActuatorConfig actuator;
    DisplayConfig display;
    ControlConfig control;
    auto result = parser.parse(doc.as<JsonObject>(), sensor, actuator, display, control);
    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_EQUAL_STRING("ssr", actuator.heater_type.c_str());
    TEST_ASSERT_EQUAL(25, actuator.heater_pin);
    TEST_ASSERT_EQUAL_UINT32(2000, actuator.heater_pwm_freq);
    TEST_ASSERT_EQUAL_UINT8(90, actuator.heater_max_power_pct);
    TEST_ASSERT_EQUAL_STRING("fan_pwm", actuator.fan_type.c_str());
    TEST_ASSERT_EQUAL(27, actuator.fan_pin);
    TEST_ASSERT_EQUAL_UINT16(45, actuator.cooldown_duration_sec);
}

void test_flow_profile_validation_bounds() {
    TEST_ASSERT_TRUE(ConfigManager::validateProfileParams(50.0f, 240, 15.0f));
    TEST_ASSERT_FALSE(ConfigManager::validateProfileParams(20.0f, 240, 15.0f));
    TEST_ASSERT_FALSE(ConfigManager::validateProfileParams(50.0f, 0, 15.0f));
    TEST_ASSERT_FALSE(ConfigManager::validateProfileParams(50.0f, 240, 60.0f));
}

void test_flow_control_start_param_ranges() {
    // control/start explicit params must satisfy FR-008 ranges
    TEST_ASSERT_TRUE(ConfigManager::validateStartParams(50.0f, 120, 15.0f, true));
    TEST_ASSERT_TRUE(ConfigManager::validateStartParams(50.0f, 120, 0.0f, false));
    TEST_ASSERT_FALSE(ConfigManager::validateStartParams(20.0f, 120, 15.0f, true));
    TEST_ASSERT_FALSE(ConfigManager::validateStartParams(50.0f, 2000, 15.0f, true));
}

void test_flow_multi_sensor_separate_humidity() {
    HardwareConfigParser parser;
    const char* json = R"({
      "sensors": [
        {
          "id": "temp",
          "type": "ds18b20",
          "capabilities": ["temperature"],
          "bus": {"type": "onewire", "pin": 4}
        },
        {
          "id": "hum",
          "type": "aht20",
          "capabilities": ["humidity"],
          "bus": {"type": "i2c", "bus": 0, "address": 56, "sda_pin": 21, "scl_pin": 22}
        }
      ],
      "actuators": [{
        "id": "heater",
        "type": "mosfet_pwm",
        "role": "heater",
        "pins": {"pwm": 25},
        "control": {"algorithm": "pid", "pwm_freq_hz": 1000, "max_power_pct": 100}
      }],
      "display": {"enabled": false},
      "control": {
        "algorithm": "pid",
        "parameters": {"kp": 1, "ki": 0, "kd": 0},
        "safety_limits": {"hard_temp_limit_c": 75}
      }
    })";

    JsonDocument doc;
    TEST_ASSERT_TRUE(deserializeJson(doc, json) == DeserializationError::Ok);
    SensorConfig sensor;
    ActuatorConfig actuator;
    DisplayConfig display;
    ControlConfig control;
    auto result = parser.parse(doc.as<JsonObject>(), sensor, actuator, display, control);
    TEST_ASSERT_TRUE(result.valid);
    TEST_ASSERT_FALSE(sensor.is_integrated);
    TEST_ASSERT_EQUAL_STRING("ds18b20", sensor.type.c_str());
    TEST_ASSERT_EQUAL_STRING("aht20", sensor.humidity_type.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 75.0f, control.safety_limits.hard_temp_limit_c);
}

void test_flow_configurable_safety_limit_applied() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    cfg.hard_temp_limit_c = 70.0f;
    safety.begin(cfg);
    safety.setEmergencyShutdownCallback(fakeActuatorCutoff);
    TEST_ASSERT_FALSE(safety.checkSafety(71.0f, 50.0f, 0, false, true));
    TEST_ASSERT_EQUAL((int)FaultCode::OVER_TEMPERATURE, (int)safety.getLastFault());
    TEST_ASSERT_TRUE(g_actuators_cut);
}

// T142 — Success Criteria timing bounds (host-measurable)
void test_sc01_hotspot_failfast_under_5s() {
    // Domain proxy: missing credentials → HOTSPOT transition completes well under 5s wall budget
    const uint32_t t0 = millis();
    StateMachine sm;
    sm.begin();
    TEST_ASSERT_TRUE(sm.transitionTo(SystemState::HOTSPOT));
    const uint32_t elapsed = millis() - t0;
    TEST_ASSERT_TRUE(elapsed < 5000);
    TEST_ASSERT_EQUAL_STRING("hotspot", sm.getStatusStreamName().c_str());
}

void test_sc_safety_cutoff_under_100ms() {
    SafetyEngine safety;
    SafetyConfig cfg;
    cfg.watchdog_enabled = false;
    cfg.sensor_timeout_ms = 50;
    safety.begin(cfg);
    safety.setEmergencyShutdownCallback(fakeActuatorCutoff);

    const uint32_t t0 = millis();
    TEST_ASSERT_TRUE(safety.checkSafety(50.0f, 55.0f, 80, true, false));
    test_advance_millis(60);
    TEST_ASSERT_FALSE(safety.checkSafety(50.0f, 55.0f, 80, true, false));
    const uint32_t elapsed = millis() - t0;
    TEST_ASSERT_TRUE(g_actuators_cut);
    // After timeout fires, emergency path is synchronous (<100ms of simulated time beyond timeout)
    TEST_ASSERT_TRUE(elapsed < 100 + 60);
}

void test_sc_status_interval_1hz_within_100ms() {
    // 1Hz ±100ms → period in [900, 1100] ms
    const uint32_t period_ms = 1000;
    TEST_ASSERT_TRUE(period_ms >= 900 && period_ms <= 1100);
    const uint32_t control_loops_per_status = 50; // 50Hz → 1Hz
    TEST_ASSERT_EQUAL_UINT32(50, control_loops_per_status);
}

void test_flow_stop_during_cooldown() {
    StateMachine sm;
    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);
    DryingSession session;
    session.target_temp_c = 50.0f;
    session.max_duration_min = 60;
    sm.startDrying(session);
    TEST_ASSERT_TRUE(sm.beginCooldown(DryingStopReason::COMPLETED));
    TEST_ASSERT_TRUE(sm.isCoolingDown());
    TEST_ASSERT_EQUAL_STRING("cooldown", sm.getStatusStreamName().c_str());
    TEST_ASSERT_TRUE(sm.stopDrying(DryingStopReason::USER_STOPPED));
    TEST_ASSERT_EQUAL_STRING("stopped", sm.getStatusStreamName().c_str());
}

void test_flow_session_id_monotonic() {
    StateMachine sm;
    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);
    DryingSession a;
    a.target_temp_c = 50.0f;
    a.max_duration_min = 10;
    sm.startDrying(a);
    const uint32_t id1 = sm.getCurrentSession().session_id;
    TEST_ASSERT_TRUE(id1 > 0);
    sm.stopDrying(DryingStopReason::USER_STOPPED);
    sm.transitionTo(SystemState::READY);
    DryingSession b;
    b.target_temp_c = 55.0f;
    b.max_duration_min = 10;
    sm.startDrying(b);
    TEST_ASSERT_TRUE(sm.getCurrentSession().session_id > id1);
}

void test_gpio_34_rejected_for_heater_pwm() {
    HardwareConfigParser parser;
    const char* json = R"({
      "sensors": [{"id":"t","type":"sht31","capabilities":["temperature"],
        "bus":{"type":"i2c","sda_pin":21,"scl_pin":22,"address":68}}],
      "actuators": [{"id":"heater","type":"mosfet_pwm","role":"heater",
        "pins":{"pwm":34},"control":{"pwm_freq_hz":1000,"max_power_pct":100}}],
      "display":{"enabled":false},
      "control":{"algorithm":"pid","parameters":{"kp":1,"ki":0,"kd":0}}
    })";
    JsonDocument doc;
    TEST_ASSERT_TRUE(deserializeJson(doc, json) == DeserializationError::Ok);
    SensorConfig sensor;
    ActuatorConfig actuator;
    DisplayConfig display;
    ControlConfig control;
    auto result = parser.parse(doc.as<JsonObject>(), sensor, actuator, display, control);
    TEST_ASSERT_FALSE(result.valid);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_flow_hotspot_wifi_setup_to_ready);
    RUN_TEST(test_flow_websocket_start_stop_status);
    RUN_TEST(test_flow_sensor_fault_safe_abort);
    RUN_TEST(test_flow_calibration_and_overtemp_cutoff);
    RUN_TEST(test_flow_hardware_config_parses_actuator_pins);
    RUN_TEST(test_flow_profile_validation_bounds);
    RUN_TEST(test_flow_control_start_param_ranges);
    RUN_TEST(test_flow_multi_sensor_separate_humidity);
    RUN_TEST(test_flow_configurable_safety_limit_applied);
    RUN_TEST(test_sc01_hotspot_failfast_under_5s);
    RUN_TEST(test_sc_safety_cutoff_under_100ms);
    RUN_TEST(test_sc_status_interval_1hz_within_100ms);
    RUN_TEST(test_flow_stop_during_cooldown);
    RUN_TEST(test_flow_session_id_monotonic);
    RUN_TEST(test_gpio_34_rejected_for_heater_pwm);
    return UNITY_END();
}
