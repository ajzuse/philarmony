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
#pragma once

#include "../UiApp.hpp"

namespace filament_dryer {
namespace ui_screens {

#if !defined(UNIT_TEST)
lv_obj_t* buildHome(UiApp& app);
lv_obj_t* buildStartPresets(UiApp& app);
lv_obj_t* buildStartCustom(UiApp& app);
lv_obj_t* buildMonitoring(UiApp& app);
lv_obj_t* buildDialogConfirm(UiApp& app);
lv_obj_t* buildTargetAdjust(UiApp& app);
lv_obj_t* buildSettingsMain(UiApp& app);
lv_obj_t* buildSettingsWifi(UiApp& app);
lv_obj_t* buildSettingsDisplay(UiApp& app);
lv_obj_t* buildSettingsUnits(UiApp& app);
lv_obj_t* buildSettingsTouch(UiApp& app);
lv_obj_t* buildSettingsAdvanced(UiApp& app);
lv_obj_t* buildSettingsSensors(UiApp& app);
lv_obj_t* buildHistoryList(UiApp& app);
lv_obj_t* buildHistoryDetail(UiApp& app);
lv_obj_t* buildDialogKeypad(UiApp& app);
#endif

} // namespace ui_screens
} // namespace filament_dryer
