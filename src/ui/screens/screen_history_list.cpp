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

lv_obj_t* buildHistoryList(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.isEnglish() ? "History" : "Historico");
    const auto records = app.historyPage();
    if (records.empty()) {
        app.addLabel(root, app.isEnglish() ? "No completed cycles"
                                           : "Nenhum ciclo concluido");
    }
    for (size_t i = 0; i < records.size(); ++i) {
        const CycleRecord& record = records[i];
        app.addButton(root,
                      "#" + String(record.id) + "  " + record.material_id +
                          "  " + String(record.duration_sec / 60) + " min  " +
                          record.stop_reason,
                      UiAction::HistorySelect, static_cast<int32_t>(i));
    }
    if (app.hasMoreHistory()) {
        app.addButton(root, app.isEnglish() ? "More" : "Mais",
                      UiAction::HistoryMore);
    }
    if (app.historyOffset() > 0) {
        app.addLabel(root, String(app.isEnglish() ? "Offset: " : "Pagina: ") +
                               String(app.historyOffset()));
    }
    app.addButton(root, app.isEnglish() ? "Back" : "Voltar",
                  UiAction::BackHome);
    return root;
}

} // namespace ui_screens
} // namespace filament_dryer
