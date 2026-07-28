#include <unity.h>
#include "Arduino.h"
#include "LittleFS.h"
#include "../../src/core/LogManager.hpp"

using namespace filament_dryer;

void setUp() {
    test_reset_littlefs();
}

void tearDown() {}

void test_system_log_write_and_read() {
    LogManager logMgr;
    TEST_ASSERT_TRUE(logMgr.begin());
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, "boot complete");

    String output;
    TEST_ASSERT_TRUE(logMgr.getSystemLog(output));
    TEST_ASSERT_TRUE(strstr(output.c_str(), "boot complete") != nullptr);
}

void test_drying_session_log_isolated() {
    LogManager logMgr;
    logMgr.begin();
    logMgr.newDryingSession();
    logMgr.logDryingStart("pla", 50.0f, 240, 15.0f);

    String output;
    TEST_ASSERT_TRUE(logMgr.getDryingLog(output));
    TEST_ASSERT_TRUE(strstr(output.c_str(), "profile=pla") != nullptr);
}

static bool callback_invoked = false;
static void onLogLine(const String&, bool) { callback_invoked = true; }

void test_log_callback_invoked() {
    LogManager logMgr;
    logMgr.begin();

    callback_invoked = false;
    logMgr.setLogCallback(onLogLine);
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, "callback test");
    TEST_ASSERT_TRUE(callback_invoked);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_system_log_write_and_read);
    RUN_TEST(test_drying_session_log_isolated);
    RUN_TEST(test_log_callback_invoked);
    return UNITY_END();
}
