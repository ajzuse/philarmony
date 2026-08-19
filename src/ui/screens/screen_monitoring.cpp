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
    app.addTitle(root, app.isPaused()
                           ? (app.isEnglish() ? "Paused" : "Pausado")
                           : (app.isEnglish() ? "Drying" : "Secando"));
    app.addLabel(root, String("Temp: ") + String(session.current_temp_c, 1) +
                           " / " + String(session.target_temp_c, 0) + " C");
    app.addLabel(root, String("RH: ") + String(session.current_humidity_pct, 1) +
                           " / " + String(session.target_humidity_pct, 0) + "%");
    app.addLabel(root, String(app.isEnglish() ? "Elapsed: " : "Decorrido: ") +
                           String(session.elapsed_sec / 60) + " min");
    app.addLabel(root, String(app.isEnglish() ? "Remaining: " : "Restante: ") +
                           String(session.remaining_sec / 60) + " min");
    app.addButton(root, app.isEnglish() ? "Adjust targets" : "Ajustar alvos",
                  UiAction::OpenTargetAdjust);
    app.addButton(root,
                  app.isPaused()
                      ? (app.isEnglish() ? "Resume" : "Retomar")
                      : (app.isEnglish() ? "Pause" : "Pausar"),
                  UiAction::PauseResume);
    app.addButton(root, app.isEnglish() ? "Stop" : "Parar",
                  UiAction::StopPrompt);
    return root;
}

} // namespace ui_screens
} // namespace filament_dryer
