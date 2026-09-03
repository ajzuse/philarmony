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

/**
 * UI theme - Dark, high-contrast RGB888 color palette + Roboto type scale
 */
#pragma once

#include <stdint.h>

#if defined(PHILARMONY_HAS_LVGL) && !defined(UNIT_TEST)
#include <lvgl.h>
#endif

namespace filament_dryer {
namespace ui_theme {

using Color = uint32_t;
static constexpr int16_t kMinTouchTargetPx = 48;

extern const Color background;
extern const Color surface;
extern const Color surface_high;
extern const Color primary;
extern const Color primary_pressed;
extern const Color secondary;
extern const Color text_primary;
extern const Color text_secondary;
extern const Color border;
extern const Color success;
extern const Color warning;
extern const Color error;

Color backgroundFor(bool high_contrast);
Color surfaceFor(bool high_contrast);
Color textFor(bool high_contrast);

#if defined(PHILARMONY_HAS_LVGL) && !defined(UNIT_TEST)
/** 12pt labels, 16pt buttons, 24pt values, 32pt titles (FR-002). */
const lv_font_t* fontLabel();
const lv_font_t* fontButton();
const lv_font_t* fontValue();
const lv_font_t* fontTitle();
#endif

} // namespace ui_theme
} // namespace filament_dryer
