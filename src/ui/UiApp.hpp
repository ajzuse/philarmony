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

/**
 * UiApp - on-device touch screen navigator (LVGL screens + controller bridge)
 */
#pragma once

#include <Arduino.h>
#include "TouchUiController.hpp"
#include "history/CycleHistoryStore.hpp"
#include "../drivers/touch/TouchManager.hpp"
#include "LvglPort.hpp"

namespace filament_dryer {

enum class UiScreenId {
    Splash,
    Home,
    StartPresets,
    StartCustom,
    Monitoring,
    SettingsMain,
    SettingsWifi,
    SettingsDisplay,
    SettingsUnits,
    SettingsTouch,
    SettingsAdvanced,
    HistoryList,
    HistoryDetail,
    DialogConfirm,
    TargetAdjust
};

class UiApp {
public:
    UiApp(TouchUiController& controller, TouchManager& touch, CycleHistoryStore& history);

    bool begin(uint16_t width, uint16_t height);
    void loop();
    void show(UiScreenId screen);
    UiScreenId current() const { return current_; }
    void setLanguage(const String& lang) { language_ = lang; }
    void notifyRemoteStateChange();
    LvglPort& lvgl() { return lvgl_; }

private:
    TouchUiController& controller_;
    TouchManager& touch_;
    CycleHistoryStore& history_;
    LvglPort lvgl_;
    UiScreenId current_ = UiScreenId::Splash;
    UiScreenId previous_ = UiScreenId::Home;
    String language_ = "pt_br";
    String pending_profile_id_;
    float draft_temp_c_ = 50.0f;
    float draft_humidity_pct_ = 15.0f;
    uint16_t draft_duration_min_ = 240;
    size_t history_offset_ = 0;
    bool confirm_is_stop_ = false;
    bool remote_toast_ = false;
    uint32_t last_timeout_check_ms_ = 0;
    bool lvgl_ready_ = false;

    void handleTouch();
    void renderStub();
};

} // namespace filament_dryer
