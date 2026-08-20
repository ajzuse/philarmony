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
#include "ui_i18n.hpp"
#include "ui_format.hpp"

#if !defined(UNIT_TEST)
#include <lvgl.h>
#endif

namespace filament_dryer {

class DisplayManager;

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
    SettingsSensors,
    HistoryList,
    HistoryDetail,
    DialogConfirm,
    DialogKeypad,
    TargetAdjust
};

enum class UiAction {
    None,
    HomeStart,
    HomeSettings,
    HomeHistory,
    BackHome,
    BackSettings,
    BackPresets,
    BackHistory,
    PresetSelect,
    PresetCustom,
    CustomTempMinus,
    CustomTempPlus,
    CustomHumidityMinus,
    CustomHumidityPlus,
    CustomDurationMinus,
    CustomDurationPlus,
    CustomStart,
    Confirm,
    Cancel,
    PauseResume,
    StopPrompt,
    OpenTargetAdjust,
    TargetTempMinus,
    TargetTempPlus,
    TargetHumidityMinus,
    TargetHumidityPlus,
    TargetDurationMinus,
    TargetDurationPlus,
    ApplyTargets,
    SettingsWifi,
    SettingsDisplay,
    SettingsUnits,
    SettingsTouch,
    SettingsAdvanced,
    SettingsSensors,
    WifiHotspot,
    BrightnessMinus,
    BrightnessPlus,
    TimeoutMinus,
    TimeoutPlus,
    OrientationNext,
    TempUnitToggle,
    LanguageToggle,
    HighContrastToggle,
    SensitivityCycle,
    CalibrationStart,
    FactoryResetPrompt,
    HistoryMore,
    HistorySelect,
    HistoryExport,
    OpenKeypad,
    KeypadDigit,
    KeypadBackspace,
    KeypadOk
};

enum class KeypadField { Temperature, Humidity, Duration };

struct WifiSnapshot {
    String ssid;
    int32_t rssi = 0;
    bool connected = false;
};

class UiApp {
public:
    using HapticCallback = void (*)();
    using WifiStatusCallback = WifiSnapshot (*)();

    UiApp(TouchUiController& controller, TouchManager& touch, CycleHistoryStore& history);

    bool begin(uint16_t width, uint16_t height,
               DisplayManager* display_manager = nullptr);
    void loop();
    void show(UiScreenId screen);
    UiScreenId current() const { return current_; }
    void setLanguage(const String& lang);
    void notifyRemoteStateChange();
    void setHapticCallback(HapticCallback callback) { haptic_callback_ = callback; }
    void setWifiStatusCallback(WifiStatusCallback callback) {
        wifi_status_callback_ = callback;
    }
    void setLiveReadings(float temp_c, float humidity_pct);
    LvglPort& lvgl() { return lvgl_; }
    bool ownsDisplay() const { return lvgl_ready_ && display_manager_; }

    void handleAction(UiAction action, int32_t value = 0);
#if !defined(UNIT_TEST)
    lv_obj_t* createScreen();
    lv_obj_t* addTitle(lv_obj_t* parent, const String& text);
    lv_obj_t* addLabel(lv_obj_t* parent, const String& text);
    lv_obj_t* addButton(lv_obj_t* parent, const String& text, UiAction action,
                        int32_t value = 0);
#endif

    const char* tr(const char* key) const {
        return ui_i18n::tr(isEnglish(), key);
    }
    std::vector<FilamentProfile> profiles() const {
        return controller_.listProfiles();
    }
    std::vector<CycleRecord> historyPage() const {
        return history_.page(history_offset_, CycleHistoryStore::kInitialPage);
    }
    const DryingSession& session() const {
        return controller_.getCurrentSession();
    }
    UISettings settings() const { return settings_; }
    bool isEnglish() const { return language_ == "en_us"; }
    bool fahrenheit() const {
        return ui_format::useFahrenheit(settings_.temp_unit);
    }
    String formatTemp(float celsius, uint8_t decimals = 1) const {
        return ui_format::formatTemp(celsius, fahrenheit(), decimals);
    }
    bool highContrast() const { return settings_.high_contrast; }
    float draftTemperature() const { return draft_temp_c_; }
    float draftHumidity() const { return draft_humidity_pct_; }
    uint16_t draftDuration() const { return draft_duration_min_; }
    bool isPaused() const { return controller_.getState() == SystemState::PAUSED; }
    bool isDrying() const { return controller_.getState() == SystemState::DRYING; }
    const char* statusKey() const;
    float liveTemperatureC() const { return live_temp_c_; }
    float liveHumidityPct() const { return live_humidity_pct_; }
    float progressPercent() const {
        return controller_.progressPercent();
    }
    size_t historyOffset() const { return history_offset_; }
    bool hasMoreHistory() const {
        return history_offset_ + CycleHistoryStore::kInitialPage < history_.size();
    }
    bool selectedHistory(CycleRecord& record) const {
        return history_.getById(selected_history_id_, record);
    }
    bool calibrationActive() const { return calibration_active_; }
    uint8_t calibrationPointIndex() const { return calibration_point_index_; }
    String confirmationText() const;
    WifiSnapshot wifiStatus() const;
    SensorConfig sensorConfig() const { return controller_.config().getSensorConfig(); }
    String firmwareVersion() const;
    String deviceName() const { return "Philarmony"; }
    float safetyTempLimitC() const { return controller_.safetyTempLimitC(); }
    String keypadBuffer() const { return keypad_buffer_; }
    KeypadField keypadField() const { return keypad_field_; }

private:
    enum class ConfirmMode { StartProfile, StartCustom, StopCycle, FactoryReset };

    TouchUiController& controller_;
    TouchManager& touch_;
    CycleHistoryStore& history_;
    LvglPort lvgl_;
    DisplayManager* display_manager_ = nullptr;
    HapticCallback haptic_callback_ = nullptr;
    WifiStatusCallback wifi_status_callback_ = nullptr;
    UiScreenId current_ = UiScreenId::Splash;
    UiScreenId previous_ = UiScreenId::Home;
    String language_ = "pt_br";
    UISettings settings_;
    String pending_profile_id_;
    float draft_temp_c_ = 50.0f;
    float draft_humidity_pct_ = 15.0f;
    uint16_t draft_duration_min_ = 240;
    float live_temp_c_ = NAN;
    float live_humidity_pct_ = NAN;
    size_t history_offset_ = 0;
    uint32_t selected_history_id_ = 0;
    ConfirmMode confirm_mode_ = ConfirmMode::StartProfile;
    KeypadField keypad_field_ = KeypadField::Temperature;
    String keypad_buffer_;
    bool remote_toast_ = false;
    uint32_t last_timeout_check_ms_ = 0;
    uint32_t last_touch_ms_ = 0;
    uint32_t last_monitor_refresh_ms_ = 0;
    uint32_t last_sync_ms_ = 0;
    uint8_t slow_frame_count_ = 0;
    bool lvgl_ready_ = false;
    bool dimmed_ = false;
    bool fallback_pressed_ = false;
    size_t fallback_profile_index_ = 0;
    String toast_text_;
    uint32_t toast_until_ms_ = 0;
    uint16_t width_ = 0;
    uint16_t height_ = 0;
    uint16_t native_width_ = 0;
    uint16_t native_height_ = 0;

    bool calibration_active_ = false;
    bool calibration_touch_down_ = false;
    uint8_t calibration_point_index_ = 0;
    TouchPoint calibration_points_[4];

#if !defined(UNIT_TEST)
    struct ActionBinding {
        UiApp* app = nullptr;
        UiAction action = UiAction::None;
        int32_t value = 0;
    };
    static constexpr size_t kMaxActionBindings = 80;
    ActionBinding action_bindings_[kMaxActionBindings];
    size_t action_binding_count_ = 0;
    lv_obj_t* root_ = nullptr;

    static void lvActionCallback(lv_event_t* event);
#endif

    void handleTouch();
    void handleFallbackTouch(const TouchPoint& point);
    void renderScreen();
    void updateInactivity();
    void playTouchFeedback();
    void showToast(const String& text, uint32_t duration_ms = 3000);
    void saveUiSettings();
    void applyTouchCalibrationToManager(const TouchConfig& config);
    void startCalibration();
    void handleCalibrationTouch();
    void finishCalibration();
    void applyOrientation();
    void syncFromSharedState();
    void openKeypad(KeypadField field);
    void applyKeypadValue();
    bool exportSelectedHistory();
    static void flushToDisplay(int16_t x1, int16_t y1, int16_t x2,
                               int16_t y2, const uint16_t* pixels,
                               void* context);
};

}  // namespace filament_dryer
