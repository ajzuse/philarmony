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

lv_obj_t* buildMonitoring(UiApp& app) {
    const DryingSession& session = app.session();
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.isPaused() ? app.tr("title_paused")
                                      : app.tr("title_drying"));

    lv_obj_t* temp = app.addLabel(
        root, app.formatTemp(session.current_temp_c) + " / " +
                  app.formatTemp(session.target_temp_c, 0));
    lv_obj_set_style_text_letter_space(temp, 1, 0);
    lv_obj_set_style_pad_ver(temp, 8, 0);
    app.addLabel(root, String(app.tr("label_rh")) + ": " +
                           String(session.current_humidity_pct, 1) + " / " +
                           String(session.target_humidity_pct, 0) + "%");
    app.addLabel(root, String(app.tr("label_elapsed")) + ": " +
                           String(session.elapsed_sec / 60) + " min");
    app.addLabel(root, String(app.tr("label_remaining")) + ": " +
                           String(session.remaining_sec / 60) + " min");

    lv_obj_t* arc = lv_arc_create(root);
    lv_obj_set_size(arc, 120, 120);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, static_cast<int16_t>(app.progressPercent()));
    lv_obj_remove_style(arc, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    app.addButton(root, app.tr("btn_adjust"), UiAction::OpenTargetAdjust);
    app.addButton(root,
                  app.isPaused() ? app.tr("btn_resume") : app.tr("btn_pause"),
                  UiAction::PauseResume);
    app.addButton(root, app.tr("btn_stop"), UiAction::StopPrompt);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
