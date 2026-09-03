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

lv_obj_t* buildStartPresets(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.tr("title_presets"));
    const auto profiles = app.profiles();
    for (size_t i = 0; i < profiles.size(); ++i) {
        const FilamentProfile& profile = profiles[i];
        const String name =
            app.isEnglish() ? profile.name_en : profile.name_pt;
        app.addButton(root,
                      name + "  " + app.formatTemp(profile.target_temp_c, 0) +
                          "  " + String(profile.default_duration_min) + " min",
                      UiAction::PresetSelect, static_cast<int32_t>(i));
    }
    app.addButton(root, app.tr("btn_custom"), UiAction::PresetCustom);
    app.addButton(root, app.tr("btn_back"), UiAction::BackHome);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
