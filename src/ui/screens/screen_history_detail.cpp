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

lv_obj_t* buildHistoryDetail(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    CycleRecord record;
    if (!app.selectedHistory(record)) {
        app.addTitle(root, app.tr("title_history"));
        app.addLabel(root, app.tr("label_not_found"));
        app.addButton(root, app.tr("btn_back"), UiAction::BackHistory);
        return root;
    }

    app.addTitle(root, "#" + String(record.id) + " [" + record.material_id +
                           "] " + ui_format::resultIcon(record.stop_reason));
    app.addLabel(root, ui_format::formatUnixDateMs(record.timestamp_ms));
    app.addLabel(root, String(app.tr("label_target")) + ": " +
                           app.formatTemp(record.target_temp_c) + " / " +
                           String(record.target_humidity_pct, 1) + "%");
    app.addLabel(root, String(app.tr("label_average")) + ": " +
                           app.formatTemp(record.avg_temp_c) + " / " +
                           String(record.avg_humidity_pct, 1) + "%");
    app.addLabel(root, String(app.tr("label_max")) + ": " +
                           app.formatTemp(record.max_temp_c));
    app.addLabel(root, String(app.tr("label_duration")) + ": " +
                           String(record.duration_sec / 60) + " min");
    app.addLabel(root, String(app.tr("label_reason")) + ": " +
                           record.stop_reason);

    const uint8_t points =
        record.sample_count > 0 ? record.sample_count : 1;
    lv_obj_t* temp_chart = lv_chart_create(root);
    lv_obj_set_size(temp_chart, LV_PCT(100), 70);
    lv_chart_set_type(temp_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(temp_chart, points);
    lv_chart_set_range(temp_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_series_t* temp_series = lv_chart_add_series(
        temp_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_obj_t* rh_chart = lv_chart_create(root);
    lv_obj_set_size(rh_chart, LV_PCT(100), 70);
    lv_chart_set_type(rh_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(rh_chart, points);
    lv_chart_set_range(rh_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_series_t* rh_series = lv_chart_add_series(
        rh_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    for (uint8_t i = 0; i < record.sample_count; ++i) {
        lv_chart_set_next_value(temp_chart, temp_series,
                                record.temp_c_x10[i] / 10);
        lv_chart_set_next_value(rh_chart, rh_series,
                                record.humidity_x10[i] / 10);
    }

    app.addButton(root, app.tr("btn_export"), UiAction::HistoryExport);
    app.addButton(root, app.tr("btn_back"), UiAction::BackHistory);
    return root;
}

}  // namespace ui_screens
}  // namespace filament_dryer
