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

#pragma once

#include <stddef.h>

namespace filament_dryer {
namespace ui_strings_en_us {

inline constexpr const char* btn_start = "Start";
inline constexpr const char* btn_stop = "Stop";
inline constexpr const char* btn_pause = "Pause";
inline constexpr const char* btn_resume = "Resume";
inline constexpr const char* btn_confirm = "Confirm";
inline constexpr const char* btn_cancel = "Cancel";
inline constexpr const char* btn_more = "More";
inline constexpr const char* title_presets = "Presets";
inline constexpr const char* title_history = "History";
inline constexpr const char* title_settings = "Settings";
inline constexpr const char* toast_pause_timeout =
    "Pause ended after 30 minutes";

struct Entry {
    const char* key;
    const char* value;
};

inline constexpr Entry table[] = {
    {"btn_start", btn_start},
    {"btn_stop", btn_stop},
    {"btn_pause", btn_pause},
    {"btn_resume", btn_resume},
    {"btn_confirm", btn_confirm},
    {"btn_cancel", btn_cancel},
    {"btn_more", btn_more},
    {"title_presets", title_presets},
    {"title_history", title_history},
    {"title_settings", title_settings},
    {"toast_pause_timeout", toast_pause_timeout},
};
inline constexpr size_t table_size = sizeof(table) / sizeof(table[0]);

} // namespace ui_strings_en_us
} // namespace filament_dryer
