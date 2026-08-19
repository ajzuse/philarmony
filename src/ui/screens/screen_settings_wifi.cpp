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

lv_obj_t* buildSettingsWifi(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, "WiFi");
    app.addLabel(root, app.isEnglish()
                           ? "Start the configuration hotspot to change network."
                           : "Inicie o hotspot para alterar a rede.");
    app.addButton(root, app.isEnglish() ? "Reconfigure" : "Reconfigurar",
                  UiAction::WifiHotspot);
    app.addButton(root, app.isEnglish() ? "Back" : "Voltar",
                  UiAction::BackSettings);
    return root;
}

} // namespace ui_screens
} // namespace filament_dryer
