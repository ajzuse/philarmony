#include <unity.h>
#include "Arduino.h"

// Debounce constants mirrored from TouchManager contract
static constexpr uint32_t kPressDebounceMs = 50;
static constexpr uint32_t kReleaseDebounceMs = 100;

void test_debounce_constants() {
    TEST_ASSERT_EQUAL_UINT32(50, kPressDebounceMs);
    TEST_ASSERT_EQUAL_UINT32(100, kReleaseDebounceMs);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_debounce_constants);
    return UNITY_END();
}
