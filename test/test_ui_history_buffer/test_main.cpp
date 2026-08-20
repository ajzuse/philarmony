/*
 * Philarmony Filament Dryer ESP32 Firmware
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <unity.h>
#include "LittleFS.h"
#include "../../src/ui/history/CycleHistoryStore.hpp"

using namespace filament_dryer;

void setUp() {
    test_reset_littlefs();
    test_set_millis(0);
}
void tearDown() {}

void test_history_persists_ring_and_pages_newest_first() {
    CycleHistoryStore store;
    TEST_ASSERT_TRUE(store.begin());
    for (uint32_t i = 0; i < 55; ++i) {
        DryingSession session;
        session.profile_id = (i % 2) ? "pla" : "petg";
        session.target_temp_c = 50.0f;
        session.target_humidity_pct = 15.0f;
        session.current_temp_c = 48.0f;
        session.current_humidity_pct = 18.0f;
        session.elapsed_sec = i;
        session.stop_reason = DryingStopReason::COMPLETED;
        TEST_ASSERT_TRUE(store.appendFromSession(session));
        test_advance_millis(1000);
    }
    TEST_ASSERT_EQUAL(CycleHistoryStore::kCapacity, store.size());

    CycleHistoryStore reopened;
    TEST_ASSERT_TRUE(reopened.begin());
    TEST_ASSERT_EQUAL(CycleHistoryStore::kCapacity, reopened.size());
    const auto first = reopened.page(0, 10);
    TEST_ASSERT_EQUAL(10, first.size());
    TEST_ASSERT_EQUAL_UINT32(54, first[0].duration_sec);
    const auto more = reopened.page(10, 10);
    TEST_ASSERT_EQUAL(10, more.size());
    TEST_ASSERT_EQUAL_UINT32(44, more[0].duration_sec);
    const auto last = reopened.page(40, 10);
    TEST_ASSERT_EQUAL(10, last.size());
    TEST_ASSERT_EQUAL_UINT32(14, last[0].duration_sec);
}

void test_history_records_samples_and_average() {
    CycleHistoryStore store;
    TEST_ASSERT_TRUE(store.begin());
    store.resetSampler();
    store.recordSample(40.0f, 20.0f);
    store.recordSample(50.0f, 10.0f);
    DryingSession session;
    session.profile_id = "pla";
    session.target_temp_c = 50.0f;
    session.target_humidity_pct = 15.0f;
    session.elapsed_sec = 120;
    session.stop_reason = DryingStopReason::COMPLETED;
    TEST_ASSERT_TRUE(store.appendFromSession(session));
    CycleRecord record;
    TEST_ASSERT_TRUE(store.getById(1, record));
    TEST_ASSERT_EQUAL(2, record.sample_count);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 45.0f, record.avg_temp_c);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, record.max_temp_c);
    TEST_ASSERT_TRUE(store.toCsv(record).indexOf("pla") >= 0);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_history_persists_ring_and_pages_newest_first);
    RUN_TEST(test_history_records_samples_and_average);
    return UNITY_END();
}
