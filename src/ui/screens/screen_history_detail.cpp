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

lv_obj_t* buildHistoryDetail(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    CycleRecord record;
    if (!app.selectedHistory(record)) {
        app.addTitle(root, app.isEnglish() ? "History detail"
                                           : "Detalhe do historico");
        app.addLabel(root, app.isEnglish() ? "Record not found"
                                           : "Registro nao encontrado");
        app.addButton(root, app.isEnglish() ? "Back" : "Voltar",
                      UiAction::BackHistory);
        return root;
    }

    app.addTitle(root, "#" + String(record.id) + " " + record.material_id);
    app.addLabel(root, String("Target: ") + String(record.target_temp_c, 1) +
                           " C / " + String(record.target_humidity_pct, 1) +
                           "% RH");
    app.addLabel(root, String("Average: ") + String(record.avg_temp_c, 1) +
                           " C / " + String(record.avg_humidity_pct, 1) +
                           "% RH");
    app.addLabel(root, String("Max: ") + String(record.max_temp_c, 1) + " C");
    app.addLabel(root, String(app.isEnglish() ? "Duration: " : "Duracao: ") +
                           String(record.duration_sec / 60) + " min");
    app.addLabel(root, String(app.isEnglish() ? "Reason: " : "Motivo: ") +
                           record.stop_reason);

    // Compact summary sparkline: target, average, maximum temperature.
    lv_obj_t* chart = lv_chart_create(root);
    lv_obj_set_size(chart, LV_PCT(100), 90);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, 3);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_series_t* series =
        lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN),
                            LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_next_value(chart, series,
                            static_cast<int32_t>(record.target_temp_c));
    lv_chart_set_next_value(chart, series,
                            static_cast<int32_t>(record.avg_temp_c));
    lv_chart_set_next_value(chart, series,
                            static_cast<int32_t>(record.max_temp_c));

    app.addButton(root, app.isEnglish() ? "Back" : "Voltar",
                  UiAction::BackHistory);
    return root;
}

} // namespace ui_screens
} // namespace filament_dryer
