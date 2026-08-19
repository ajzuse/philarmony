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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "UiApp.hpp"
#include "assets/ui_strings_pt_br.hpp"
#include "assets/ui_strings_en_us.hpp"

namespace filament_dryer {

UiApp::UiApp(TouchUiController& controller, TouchManager& touch, CycleHistoryStore& history)
    : controller_(controller), touch_(touch), history_(history) {}

bool UiApp::begin(uint16_t width, uint16_t height) {
    lvgl_ready_ = lvgl_.init(touch_, width, height, nullptr, nullptr);
    show(UiScreenId::Home);
    return true;
}

void UiApp::show(UiScreenId screen) {
    previous_ = current_;
    current_ = screen;
    renderStub();
}

void UiApp::notifyRemoteStateChange() {
    remote_toast_ = true;
}

void UiApp::loop() {
    controller_.tick();
    handleTouch();
    if (lvgl_ready_) {
        lvgl_.tick();
    }
    if (millis() - last_timeout_check_ms_ > 500) {
        last_timeout_check_ms_ = millis();
    }
}

void UiApp::handleTouch() {
    TouchPoint p = touch_.read();
    if (!p.pressed) {
        return;
    }

    // Simplified region map for ≥240x240: bottom third = primary actions
    const bool lower = p.y > 160;
    const bool left = p.x < 120;
    const bool right = p.x >= 120;

    switch (current_) {
        case UiScreenId::Home:
            if (lower && left) {
                show(UiScreenId::StartPresets);
            } else if (lower && right) {
                show(UiScreenId::SettingsMain);
            } else if (p.y < 80 && right) {
                history_offset_ = 0;
                show(UiScreenId::HistoryList);
            }
            break;
        case UiScreenId::StartPresets:
            if (lower && left) {
                pending_profile_id_ = "pla";
                confirm_is_stop_ = false;
                show(UiScreenId::DialogConfirm);
            } else if (lower && right) {
                show(UiScreenId::StartCustom);
            } else if (p.y < 40) {
                show(UiScreenId::Home);
            }
            break;
        case UiScreenId::StartCustom:
            if (lower) {
                controller_.startCustom(draft_temp_c_, draft_humidity_pct_, draft_duration_min_);
                show(UiScreenId::Monitoring);
            } else if (p.y < 40) {
                show(UiScreenId::StartPresets);
            }
            break;
        case UiScreenId::DialogConfirm:
            if (lower && left) {
                if (confirm_is_stop_) {
                    controller_.stop();
                    show(UiScreenId::Home);
                } else {
                    controller_.startFromProfile(pending_profile_id_);
                    show(UiScreenId::Monitoring);
                }
            } else if (lower && right) {
                show(previous_);
            }
            break;
        case UiScreenId::Monitoring:
            if (lower && left) {
                // pause / resume toggle via repeated taps on left
                if (!controller_.pause()) {
                    controller_.resume();
                }
            } else if (lower && right) {
                confirm_is_stop_ = true;
                show(UiScreenId::DialogConfirm);
            } else if (p.y > 80 && p.y < 140) {
                show(UiScreenId::TargetAdjust);
            }
            break;
        case UiScreenId::TargetAdjust:
            if (lower) {
                controller_.applyTargets(draft_temp_c_, draft_humidity_pct_);
                show(UiScreenId::Monitoring);
            } else if (left) {
                draft_temp_c_ = (draft_temp_c_ > 30.0f) ? draft_temp_c_ - 1.0f : 30.0f;
            } else {
                draft_temp_c_ = (draft_temp_c_ < 80.0f) ? draft_temp_c_ + 1.0f : 80.0f;
            }
            break;
        case UiScreenId::SettingsMain:
            if (p.y < 40) {
                show(UiScreenId::Home);
            } else if (p.y < 80) {
                show(UiScreenId::SettingsWifi);
            } else if (p.y < 120) {
                show(UiScreenId::SettingsDisplay);
            } else if (p.y < 160) {
                show(UiScreenId::SettingsUnits);
            } else if (p.y < 200) {
                show(UiScreenId::SettingsTouch);
            } else {
                show(UiScreenId::SettingsAdvanced);
            }
            break;
        case UiScreenId::SettingsWifi:
        case UiScreenId::SettingsDisplay:
        case UiScreenId::SettingsUnits:
        case UiScreenId::SettingsTouch:
        case UiScreenId::SettingsAdvanced:
            if (p.y < 40) {
                show(UiScreenId::SettingsMain);
            }
            break;
        case UiScreenId::HistoryList:
            if (p.y < 40) {
                show(UiScreenId::Home);
            } else if (lower) {
                history_offset_ += CycleHistoryStore::kInitialPage;
                show(UiScreenId::HistoryList);
            } else {
                show(UiScreenId::HistoryDetail);
            }
            break;
        case UiScreenId::HistoryDetail:
            if (p.y < 40) {
                show(UiScreenId::HistoryList);
            }
            break;
        default:
            show(UiScreenId::Home);
            break;
    }
    (void)language_;
    (void)remote_toast_;
}

void UiApp::renderStub() {
    // LVGL widget trees expand incrementally; navigator drives control flows today.
}

} // namespace filament_dryer
