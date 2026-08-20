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
#include "../ui_format.hpp"

namespace filament_dryer {
namespace ui_screens {

lv_obj_t* buildHistoryList(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.tr("title_history"));
    const auto records = app.historyPage();
    if (records.empty()) {
        app.addLabel(root, app.tr("label_no_history"));
    }
    for (size_t i = 0; i < records.size(); ++i) {
        const CycleRecord& record = records[i];
        const String line =
            ui_format::formatUnixDateMs(record.timestamp_ms) + "  [" +
            record.material_id + "]  " +
            app.formatTemp(record.target_temp_c, 0) + "  " +
            String(record.duration_sec / 60) + " min  " +
            ui_format::resultIcon(record.stop_reason);
        app.addButton(root, line, UiAction::HistorySelect,
                      static_cast<int32_t>(i));
    }
    if (app.hasMoreHistory()) {
        app.addButton(root, app.tr("btn_more"), UiAction::HistoryMore);
    }
    app.addButton(root, app.tr("btn_back"), UiAction::BackHome);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
