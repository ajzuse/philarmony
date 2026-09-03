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
#include <cmath>

namespace filament_dryer {
namespace ui_screens {

lv_obj_t* buildHome(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, "Philarmony");
    app.addLabel(root, String(app.tr("label_status")) + ": " +
                           app.tr(app.statusKey()));
    const float temp = std::isfinite(app.liveTemperatureC())
                           ? app.liveTemperatureC()
                           : app.session().current_temp_c;
    const float rh = std::isfinite(app.liveHumidityPct())
                         ? app.liveHumidityPct()
                         : app.session().current_humidity_pct;
    app.addLabel(root, String(app.tr("label_temp")) + ": " +
                           app.formatTemp(temp));
    app.addLabel(root, String(app.tr("label_rh")) + ": " +
                           String(rh, 1) + "%");
    app.addButton(root, app.tr("btn_start"), UiAction::HomeStart);
    app.addButton(root, app.tr("btn_history"), UiAction::HomeHistory);
    app.addButton(root, app.tr("btn_settings"), UiAction::HomeSettings);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
