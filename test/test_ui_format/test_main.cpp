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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <unity.h>
#include "../../src/ui/ui_format.hpp"
#include "../../src/ui/ui_i18n.hpp"

using namespace filament_dryer;

void setUp() {}
void tearDown() {}

void test_fahrenheit_conversion_and_suffix() {
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 122.0f,
                             ui_format::toDisplayTemp(50.0f, true));
    TEST_ASSERT_TRUE(ui_format::formatTemp(50.0f, true, 0).indexOf("F") >= 0);
    TEST_ASSERT_TRUE(ui_format::formatTemp(50.0f, false, 0).indexOf("C") >= 0);
}

void test_result_icon_and_rssi_bars() {
    TEST_ASSERT_EQUAL_STRING("✓", ui_format::resultIcon("completed"));
    TEST_ASSERT_EQUAL_STRING("✗", ui_format::resultIcon("safety_cutoff"));
    TEST_ASSERT_EQUAL_STRING("⚠", ui_format::resultIcon("user_stopped"));
    TEST_ASSERT_EQUAL_STRING("----", ui_format::rssiBars(-40, false).c_str());
    TEST_ASSERT_EQUAL_STRING("████", ui_format::rssiBars(-40, true).c_str());
}

void test_i18n_tables_resolve_start_label() {
    TEST_ASSERT_EQUAL_STRING("Iniciar Secagem", ui_i18n::tr(false, "btn_start"));
    TEST_ASSERT_EQUAL_STRING("Start Drying", ui_i18n::tr(true, "btn_start"));
}

void test_orientation_mapping_and_active_cycle() {
    TEST_ASSERT_EQUAL_UINT8(1, ui_format::orientationToLgfx(90));
    TEST_ASSERT_TRUE(ui_format::isActiveCycle(SystemState::DRYING));
    TEST_ASSERT_FALSE(ui_format::isActiveCycle(SystemState::READY));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_fahrenheit_conversion_and_suffix);
    RUN_TEST(test_result_icon_and_rssi_bars);
    RUN_TEST(test_i18n_tables_resolve_start_label);
    RUN_TEST(test_orientation_mapping_and_active_cycle);
    return UNITY_END();
}
