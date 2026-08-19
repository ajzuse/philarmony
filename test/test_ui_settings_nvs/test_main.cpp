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
#include "Preferences.h"
#include "../../src/core/ConfigManager.hpp"

using namespace filament_dryer;

void setUp() { test_reset_preferences(); }
void tearDown() {}

void test_touch_and_ui_settings_survive_manager_reopen() {
    {
        ConfigManager config;
        TEST_ASSERT_TRUE(config.begin());
        TouchConfig touch;
        touch.controller_type = "xpt2046";
        touch.spi_cs = 33;
        touch.irq_pin = 36;
        touch.spi_mosi = 32;
        touch.spi_miso = 39;
        touch.spi_sclk = 25;
        touch.calibration = {210, 3770, 240, 3740, true};
        touch.sensitivity = "high";
        touch.swap_xy = true;
        touch.invert_y = true;
        TEST_ASSERT_TRUE(config.setTouchConfig(touch));

        UISettings ui;
        ui.brightness_pct = 60;
        ui.timeout_sec = 300;
        ui.orientation = 90;
        ui.temp_unit = "fahrenheit";
        ui.language = "en_us";
        ui.touch_sensitivity = "high";
        ui.high_contrast = true;
        ui.pin_lock_enabled = true;
        ui.pin_hash = "sha256:test";
        TEST_ASSERT_TRUE(config.setUiSettings(ui));
    }

    ConfigManager reopened;
    TEST_ASSERT_TRUE(reopened.begin());
    const TouchConfig touch = reopened.getTouchConfig();
    TEST_ASSERT_EQUAL_STRING("xpt2046", touch.controller_type.c_str());
    TEST_ASSERT_EQUAL_INT(33, touch.spi_cs);
    TEST_ASSERT_EQUAL_INT(39, touch.spi_miso);
    TEST_ASSERT_EQUAL_INT(210, touch.calibration.x_min);
    TEST_ASSERT_TRUE(touch.calibration.swapped_xy);
    TEST_ASSERT_TRUE(touch.invert_y);

    const UISettings ui = reopened.getUiSettings();
    TEST_ASSERT_EQUAL_UINT8(60, ui.brightness_pct);
    TEST_ASSERT_EQUAL_UINT16(300, ui.timeout_sec);
    TEST_ASSERT_EQUAL_UINT16(90, ui.orientation);
    TEST_ASSERT_EQUAL_STRING("fahrenheit", ui.temp_unit.c_str());
    TEST_ASSERT_EQUAL_STRING("en_us", ui.language.c_str());
    TEST_ASSERT_TRUE(ui.high_contrast);
    TEST_ASSERT_EQUAL_STRING("sha256:test", ui.pin_hash.c_str());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_touch_and_ui_settings_survive_manager_reopen);
    return UNITY_END();
}
