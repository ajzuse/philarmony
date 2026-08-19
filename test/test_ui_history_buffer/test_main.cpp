#include <unity.h>
#include "Arduino.h"
#include "../../src/ui/history/CycleHistoryStore.hpp"
#include "../../src/core/StateMachine.hpp"

using namespace filament_dryer;

void setUp() { test_set_millis(0); }
void tearDown() {}

void test_history_ring_and_page() {
    CycleHistoryStore store;
    TEST_ASSERT_TRUE(store.begin());

    for (uint32_t i = 0; i < 12; ++i) {
        DryingSession s;
        s.profile_id = String("pla");
        s.target_temp_c = 50.0f;
        s.elapsed_sec = i;
        s.stop_reason = DryingStopReason::COMPLETED;
        store.appendFromSession(s);
        test_advance_millis(1000);
    }

    TEST_ASSERT_EQUAL(12, store.size());
    auto page = store.page(0, 10);
    TEST_ASSERT_EQUAL(10, page.size());
    TEST_ASSERT_EQUAL_UINT32(11, page[0].duration_sec); // newest

    auto more = store.page(10, 10);
    TEST_ASSERT_EQUAL(2, more.size());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_history_ring_and_page);
    return UNITY_END();
}
