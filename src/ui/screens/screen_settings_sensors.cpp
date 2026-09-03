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

lv_obj_t* buildSettingsSensors(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.tr("title_sensors"));
    app.addLabel(root, app.tr("sensors_readonly"));
    const SensorConfig sensors = app.sensorConfig();
    app.addLabel(root, "T: " + sensors.type + "  0x" +
                           String(sensors.i2c_address, HEX) +
                           "  SDA " + String(sensors.sda_pin) +
                           " SCL " + String(sensors.scl_pin));
    if (!sensors.humidity_type.isEmpty()) {
        app.addLabel(root, "RH: " + sensors.humidity_type + "  0x" +
                               String(sensors.humidity_i2c_address, HEX));
    }
    if (!sensors.extra_temp_type.isEmpty()) {
        app.addLabel(root, "T2: " + sensors.extra_temp_type);
    }
    app.addButton(root, app.tr("btn_back"), UiAction::BackSettings);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
