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

#include "ui_theme.hpp"
#if defined(PHILARMONY_HAS_LVGL) && !defined(UNIT_TEST)
#include "../assets/ui_fonts.hpp"
#endif

namespace filament_dryer {
namespace ui_theme {

const Color background = 0x0B0F14;
const Color surface = 0x151B23;
const Color surface_high = 0x202936;
const Color primary = 0x00C853;
const Color primary_pressed = 0x00963E;
const Color secondary = 0x40C4FF;
const Color text_primary = 0xFFFFFF;
const Color text_secondary = 0xB8C1CC;
const Color border = 0x465364;
const Color success = 0x69F0AE;
const Color warning = 0xFFD740;
const Color error = 0xFF5252;

Color backgroundFor(bool high_contrast) {
    return high_contrast ? 0x000000 : background;
}

Color surfaceFor(bool high_contrast) {
    return high_contrast ? 0x101010 : surface;
}

Color textFor(bool high_contrast) {
    return high_contrast ? 0xFFFFFF : text_primary;
}

#if defined(PHILARMONY_HAS_LVGL) && !defined(UNIT_TEST)
const lv_font_t* fontLabel() { return &ui_font_roboto_12; }
const lv_font_t* fontButton() { return &ui_font_roboto_16; }
const lv_font_t* fontValue() { return &ui_font_roboto_24; }
const lv_font_t* fontTitle() { return &ui_font_roboto_32; }
#endif

} // namespace ui_theme
} // namespace filament_dryer
