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

lv_obj_t* buildSettingsDisplay(UiApp& app) {
    const UISettings settings = app.settings();
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.tr("title_display"));
    app.addLabel(root, String(app.tr("label_brightness")) + ": " +
                           String(settings.brightness_pct) + "%");
    app.addButton(root, "- 10%", UiAction::BrightnessMinus);
    app.addButton(root, "+ 10%", UiAction::BrightnessPlus);
    app.addLabel(root, String(app.tr("label_timeout")) + ": " +
                           String(settings.timeout_sec) + " s");
    app.addButton(root, "- 30 s", UiAction::TimeoutMinus);
    app.addButton(root, "+ 30 s", UiAction::TimeoutPlus);
    app.addButton(root, String(app.tr("label_orientation")) + ": " +
                            String(settings.orientation),
                  UiAction::OrientationNext);
    app.addButton(root, app.tr("btn_back"), UiAction::BackSettings);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
