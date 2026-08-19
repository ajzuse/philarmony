#include <unity.h>

static bool validateStartParams(float temp_c, uint16_t duration_min, float humidity_pct) {
    if (temp_c < 30.0f || temp_c > 80.0f) return false;
    if (duration_min < 1 || duration_min > 1440) return false;
    if (humidity_pct < 5.0f || humidity_pct > 50.0f) return false;
    return true;
}

void test_custom_start_ranges() {
    TEST_ASSERT_TRUE(validateStartParams(50.0f, 240, 15.0f));
    TEST_ASSERT_FALSE(validateStartParams(20.0f, 240, 15.0f));
    TEST_ASSERT_FALSE(validateStartParams(50.0f, 0, 15.0f));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_custom_start_ranges);
    return UNITY_END();
}
