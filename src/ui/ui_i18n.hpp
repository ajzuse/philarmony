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

#pragma once

#include <cstring>

#include "assets/ui_strings_en_us.hpp"
#include "assets/ui_strings_pt_br.hpp"

namespace filament_dryer {
namespace ui_i18n {

inline const char* tr(bool english, const char* key) {
    if (key == nullptr) {
        return "";
    }
    if (english) {
        for (size_t i = 0; i < ui_strings_en_us::table_size; ++i) {
            if (strcmp(ui_strings_en_us::table[i].key, key) == 0) {
                return ui_strings_en_us::table[i].value;
            }
        }
    } else {
        for (size_t i = 0; i < ui_strings_pt_br::table_size; ++i) {
            if (strcmp(ui_strings_pt_br::table[i].key, key) == 0) {
                return ui_strings_pt_br::table[i].value;
            }
        }
    }
    return key;
}

}  // namespace ui_i18n
}  // namespace filament_dryer
