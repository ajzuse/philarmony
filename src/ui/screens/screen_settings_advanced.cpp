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
#include "ScreenBuilders.hpp"

namespace filament_dryer {
namespace ui_screens {

lv_obj_t* buildSettingsAdvanced(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.isEnglish() ? "Advanced" : "Avancado");
    app.addLabel(root, app.isEnglish()
                           ? "Sensors are configured by the installer."
                           : "Sensores sao configurados pelo instalador.");
    app.addButton(root, app.isEnglish() ? "Factory reset"
                                        : "Restaurar configuracoes",
                  UiAction::FactoryResetPrompt);
    app.addButton(root, app.isEnglish() ? "Back" : "Voltar",
                  UiAction::BackSettings);
    return root;
}

} // namespace ui_screens
} // namespace filament_dryer
