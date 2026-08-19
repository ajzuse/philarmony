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

lv_obj_t* buildSettingsTouch(UiApp& app) {
    lv_obj_t* root = app.createScreen();
    app.addTitle(root, app.isEnglish() ? "Touch" : "Toque");
    if (app.calibrationActive()) {
        static const char* points_pt[] = {
            "superior esquerdo", "superior direito",
            "inferior direito", "inferior esquerdo"};
        static const char* points_en[] = {
            "top left", "top right", "bottom right", "bottom left"};
        const uint8_t index = app.calibrationPointIndex();
        app.addLabel(root, String(app.isEnglish() ? "Touch target: "
                                                  : "Toque no alvo: ") +
                               (app.isEnglish() ? points_en[index]
                                                : points_pt[index]));
        app.addLabel(root, "+");
        return root;
    }

    app.addButton(root,
                  String(app.isEnglish() ? "Sensitivity: "
                                         : "Sensibilidade: ") +
                      app.settings().touch_sensitivity,
                  UiAction::SensitivityCycle);
    app.addButton(root, app.isEnglish() ? "Calibrate (4 points)"
                                        : "Calibrar (4 pontos)",
                  UiAction::CalibrationStart);
    app.addButton(root, app.isEnglish() ? "Back" : "Voltar",
                  UiAction::BackSettings);
    return root;
}

} // namespace ui_screens
} // namespace filament_dryer
