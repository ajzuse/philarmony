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
    app.addTitle(root, app.tr("title_adjust"));
    app.addLabel(root, String(app.tr("label_temp")) + ": " +
                           app.formatTemp(app.draftTemperature(), 0));
    app.addButton(root, "- 1", UiAction::TargetTempMinus);
    app.addButton(root, "+ 1", UiAction::TargetTempPlus);
    app.addButton(root, app.tr("btn_keypad"), UiAction::OpenKeypad,
                  static_cast<int32_t>(KeypadField::Temperature));
    app.addLabel(root, String(app.tr("label_rh")) + ": " +
                           String(app.draftHumidity(), 0) + "%");
    app.addButton(root, "- 1% RH", UiAction::TargetHumidityMinus);
    app.addButton(root, "+ 1% RH", UiAction::TargetHumidityPlus);
    app.addButton(root, app.tr("btn_keypad"), UiAction::OpenKeypad,
                  static_cast<int32_t>(KeypadField::Humidity));
    app.addLabel(root, String(app.tr("label_duration")) + ": " +
                           String(app.draftDuration()) + " min");
    app.addButton(root, "- 5 min", UiAction::TargetDurationMinus);
    app.addButton(root, "+ 5 min", UiAction::TargetDurationPlus);
    app.addButton(root, app.tr("btn_keypad"), UiAction::OpenKeypad,
                  static_cast<int32_t>(KeypadField::Duration));
    app.addButton(root, app.tr("btn_apply"), UiAction::ApplyTargets);
    app.addButton(root, app.tr("btn_cancel"), UiAction::Cancel);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
