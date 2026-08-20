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
    app.addTitle(root, app.tr("title_advanced"));
    app.addLabel(root, String(app.tr("label_device")) + ": " +
                           app.deviceName());
    app.addLabel(root, String(app.tr("label_firmware")) + ": " +
                           app.firmwareVersion());
    app.addLabel(root, String(app.tr("label_safety")) + ": " +
                           app.formatTemp(app.safetyTempLimitC(), 0));
    app.addButton(root, app.tr("btn_reset"), UiAction::FactoryResetPrompt);
    app.addButton(root, app.tr("btn_back"), UiAction::BackSettings);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
