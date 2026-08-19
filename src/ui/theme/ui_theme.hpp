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
 * UI theme - Dark, high-contrast RGB888 color palette
 */
#pragma once

#include <stdint.h>

namespace filament_dryer {
namespace ui_theme {

using Color = uint32_t;

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

} // namespace ui_theme
} // namespace filament_dryer
