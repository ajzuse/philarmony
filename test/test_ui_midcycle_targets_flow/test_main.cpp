#include <unity.h>
#include "Arduino.h"
#include "../../src/core/StateMachine.hpp"

using namespace filament_dryer;

void test_apply_targets_while_drying() {
    StateMachine sm;
    sm.begin();
    sm.transitionTo(SystemState::WIFI_CONNECT);
    sm.transitionTo(SystemState::READY);
    DryingSession s;
    s.target_temp_c = 50;
    s.target_humidity_pct = 15;
    s.max_duration_min = 60;
    sm.startDrying(s);
    auto& sess = sm.getCurrentSession();
    sess.target_temp_c = 55.0f;
    sess.target_humidity_pct = 12.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 55.0f, sm.getCurrentSession().target_temp_c);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_apply_targets_while_drying);
    return UNITY_END();
}
