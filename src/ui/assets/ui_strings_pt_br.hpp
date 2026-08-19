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
namespace ui_strings_pt_br {

inline constexpr const char* btn_start = "Iniciar";
inline constexpr const char* btn_stop = "Parar";
inline constexpr const char* btn_pause = "Pausar";
inline constexpr const char* btn_resume = "Retomar";
inline constexpr const char* btn_confirm = "Confirmar";
inline constexpr const char* btn_cancel = "Cancelar";
inline constexpr const char* btn_more = "Mais";
inline constexpr const char* title_presets = "Predefinições";
inline constexpr const char* title_history = "Histórico";
inline constexpr const char* title_settings = "Configurações";
inline constexpr const char* toast_pause_timeout =
    "Pausa encerrada após 30 minutos";

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

} // namespace ui_strings_pt_br
} // namespace filament_dryer
