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

lv_obj_t* buildTargetAdjust(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.isEnglish() ? "Adjust targets" : "Ajustar alvos");
    app.addLabel(root, String("Temp: ") + String(app.draftTemperature(), 0) + " C");
    app.addButton(root, "- 1 C", UiAction::TargetTempMinus);
    app.addButton(root, "+ 1 C", UiAction::TargetTempPlus);
    app.addLabel(root, String("RH: ") + String(app.draftHumidity(), 0) + "%");
    app.addButton(root, "- 1% RH", UiAction::TargetHumidityMinus);
    app.addButton(root, "+ 1% RH", UiAction::TargetHumidityPlus);
    app.addLabel(root, String(app.isEnglish() ? "Duration: " : "Duracao: ") +
                           String(app.draftDuration()) + " min");
    app.addButton(root, "- 5 min", UiAction::TargetDurationMinus);
    app.addButton(root, "+ 5 min", UiAction::TargetDurationPlus);
    app.addButton(root, app.isEnglish() ? "Apply" : "Aplicar",
                  UiAction::ApplyTargets);
    app.addButton(root, app.isEnglish() ? "Cancel" : "Cancelar",
                  UiAction::Cancel);
    return root;
}

} // namespace ui_screens
} // namespace filament_dryer
