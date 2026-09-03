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

lv_obj_t* buildStartCustom(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.tr("title_custom"));
    app.addLabel(root, String(app.tr("label_temp")) + ": " +
                           app.formatTemp(app.draftTemperature(), 0));
    app.addButton(root, "- 1", UiAction::CustomTempMinus);
    app.addButton(root, "+ 1", UiAction::CustomTempPlus);
    app.addButton(root, app.tr("btn_keypad"), UiAction::OpenKeypad,
                  static_cast<int32_t>(KeypadField::Temperature));
    app.addLabel(root, String(app.tr("label_rh")) + ": " +
                           String(app.draftHumidity(), 0) + "%");
    app.addButton(root, "- 1% RH", UiAction::CustomHumidityMinus);
    app.addButton(root, "+ 1% RH", UiAction::CustomHumidityPlus);
    app.addButton(root, app.tr("btn_keypad"), UiAction::OpenKeypad,
                  static_cast<int32_t>(KeypadField::Humidity));
    app.addLabel(root, String(app.tr("label_duration")) + ": " +
                           String(app.draftDuration()) + " min");
    app.addButton(root, "- 5 min", UiAction::CustomDurationMinus);
    app.addButton(root, "+ 5 min", UiAction::CustomDurationPlus);
    app.addButton(root, app.tr("btn_keypad"), UiAction::OpenKeypad,
                  static_cast<int32_t>(KeypadField::Duration));
    app.addButton(root, app.tr("btn_confirm"), UiAction::CustomStart);
    app.addButton(root, app.tr("btn_back"), UiAction::BackPresets);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
