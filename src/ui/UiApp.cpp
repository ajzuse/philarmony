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

#include "UiApp.hpp"

#include "screens/ScreenBuilders.hpp"
#include "theme/ui_theme.hpp"
#include "../drivers/display/DisplayManager.hpp"
#include "firmware_version.h"

#include <algorithm>
#include <cmath>

namespace filament_dryer {

UiApp::UiApp(TouchUiController& controller, TouchManager& touch,
             CycleHistoryStore& history)
    : controller_(controller), touch_(touch), history_(history) {}

bool UiApp::begin(uint16_t width, uint16_t height,
                  DisplayManager* display_manager) {
    width_ = width;
    height_ = height;
    native_width_ = width;
    native_height_ = height;
    display_manager_ =
        display_manager && display_manager->getActiveDriver()
            ? display_manager
            : nullptr;
    settings_ = controller_.getUiSettings();
    language_ = settings_.language;
    last_touch_ms_ = millis();
    applyTouchCalibrationToManager(controller_.getTouchConfig());

    if (display_manager_) {
        display_manager_->setBrightness(
            static_cast<uint8_t>((settings_.brightness_pct * 255u) / 100u));
        applyOrientation();
        lvgl_ready_ = lvgl_.init(touch_, width_, height_, flushToDisplay,
                                 display_manager_);
    }
    if (ui_format::isActiveCycle(controller_.getState())) {
        show(UiScreenId::Monitoring);
    } else {
        show(UiScreenId::Home);
    }
    return lvgl_ready_;
}

void UiApp::show(UiScreenId screen) {
    if (screen != current_) {
        previous_ = current_;
    }
    current_ = screen;
    renderScreen();
}

void UiApp::setLanguage(const String& lang) {
    language_ = lang == "en_us" ? "en_us" : "pt_br";
}

void UiApp::notifyRemoteStateChange() {
    remote_toast_ = true;
    showToast(tr("toast_remote"));
    syncFromSharedState();
    renderScreen();
}

void UiApp::setLiveReadings(float temp_c, float humidity_pct) {
    live_temp_c_ = temp_c;
    live_humidity_pct_ = humidity_pct;
}

const char* UiApp::statusKey() const {
    switch (controller_.getState()) {
        case SystemState::DRYING:
            return "status_drying";
        case SystemState::PAUSED:
            return "status_paused";
        case SystemState::STOPPED:
        case SystemState::FAULT_STOPPED:
            return "status_stopped";
        default:
            return "status_idle";
    }
}

WifiSnapshot UiApp::wifiStatus() const {
    if (wifi_status_callback_) {
        return wifi_status_callback_();
    }
    WifiSnapshot snapshot;
    snapshot.ssid = controller_.config().getWifiConfig().ssid;
    return snapshot;
}

String UiApp::firmwareVersion() const {
    return FIRMWARE_VERSION;
}

void UiApp::loop() {
    const uint32_t frame_start = millis();
    controller_.tick();
    touch_.serviceWatchdog();
    if (controller_.consumePauseTimeoutEvent()) {
        showToast(tr("toast_pause_timeout"));
        show(UiScreenId::Home);
    }

    if (millis() - last_sync_ms_ >= 250) {
        last_sync_ms_ = millis();
        syncFromSharedState();
    }

    if (calibration_active_) {
        handleCalibrationTouch();
    } else if (lvgl_ready_) {
        lvgl_.tick();
        if (lvgl_.lastTouchMs() > last_touch_ms_) {
            last_touch_ms_ = lvgl_.lastTouchMs();
        }
    } else {
        handleTouch();
    }

    if (current_ == UiScreenId::Monitoring &&
        millis() - last_monitor_refresh_ms_ >= 1000) {
        last_monitor_refresh_ms_ = millis();
        renderScreen();
    }

    if (millis() - last_timeout_check_ms_ >= 500) {
        last_timeout_check_ms_ = millis();
        updateInactivity();
    }

    const uint32_t frame_ms = millis() - frame_start;
    if (frame_ms > 100) {
        ++slow_frame_count_;
        if (slow_frame_count_ >= 5 && display_manager_) {
            lvgl_.deinit();
            lvgl_ready_ = lvgl_.init(touch_, width_, height_, flushToDisplay,
                                     display_manager_);
            slow_frame_count_ = 0;
            renderScreen();
        }
    } else {
        slow_frame_count_ = 0;
    }
}

void UiApp::handleTouch() {
    const TouchPoint point = touch_.read();
    if (!point.pressed) {
        fallback_pressed_ = false;
        return;
    }
    last_touch_ms_ = point.timestamp_ms ? point.timestamp_ms : millis();
    if (fallback_pressed_) return;
    fallback_pressed_ = true;
    handleFallbackTouch(point);
}

void UiApp::handleFallbackTouch(const TouchPoint& point) {
    const int16_t half_w = static_cast<int16_t>(width_ / 2);
    const int16_t top_h = 48;
    const int16_t lower_y = static_cast<int16_t>((height_ * 2) / 3);
    const bool lower = point.y >= lower_y;
    const bool left = point.x < half_w;

    switch (current_) {
        case UiScreenId::Home:
            if (lower && left) handleAction(UiAction::HomeStart);
            else if (lower) handleAction(UiAction::HomeSettings);
            else if (point.y < height_ / 3 && !left)
                handleAction(UiAction::HomeHistory);
            break;
        case UiScreenId::StartPresets:
            if (point.y < top_h) {
                handleAction(UiAction::BackHome);
            } else if (lower && left) {
                handleAction(UiAction::PresetSelect,
                             static_cast<int32_t>(fallback_profile_index_));
            } else if (lower) {
                handleAction(UiAction::PresetCustom);
            } else {
                const auto items = profiles();
                if (!items.empty()) {
                    fallback_profile_index_ =
                        (fallback_profile_index_ + 1) % items.size();
                    showToast(isEnglish()
                                  ? items[fallback_profile_index_].name_en
                                  : items[fallback_profile_index_].name_pt,
                              1200);
                }
            }
            break;
        case UiScreenId::StartCustom:
            if (point.y < top_h) {
                handleAction(UiAction::BackPresets);
            } else {
                const int32_t band =
                    ((point.y - top_h) * 4) /
                    std::max<int16_t>(1, height_ - top_h);
                if (band == 0)
                    handleAction(left ? UiAction::CustomTempMinus
                                      : UiAction::CustomTempPlus);
                else if (band == 1)
                    handleAction(left ? UiAction::CustomHumidityMinus
                                      : UiAction::CustomHumidityPlus);
                else if (band == 2)
                    handleAction(left ? UiAction::CustomDurationMinus
                                      : UiAction::CustomDurationPlus);
                else
                    handleAction(UiAction::CustomStart);
            }
            break;
        case UiScreenId::DialogConfirm:
            if (lower)
                handleAction(left ? UiAction::Confirm : UiAction::Cancel);
            break;
        case UiScreenId::Monitoring:
            if (lower && left) handleAction(UiAction::PauseResume);
            else if (lower) handleAction(UiAction::StopPrompt);
            else handleAction(UiAction::OpenTargetAdjust);
            break;
        case UiScreenId::TargetAdjust:
            {
                const int32_t band =
                    (point.y * 4) / std::max<uint16_t>(1, height_);
                if (band == 0)
                    handleAction(left ? UiAction::TargetTempMinus
                                      : UiAction::TargetTempPlus);
                else if (band == 1)
                    handleAction(left ? UiAction::TargetHumidityMinus
                                      : UiAction::TargetHumidityPlus);
                else if (band == 2)
                    handleAction(left ? UiAction::TargetDurationMinus
                                      : UiAction::TargetDurationPlus);
                else
                    handleAction(UiAction::ApplyTargets);
            }
            break;
        case UiScreenId::SettingsMain:
            if (point.y < top_h) {
                handleAction(UiAction::BackHome);
            } else {
                const int32_t row =
                    ((point.y - top_h) * 6) /
                    std::max<int16_t>(1, height_ - top_h);
                const UiAction actions[] = {
                    UiAction::SettingsWifi, UiAction::SettingsDisplay,
                    UiAction::SettingsUnits, UiAction::SettingsTouch,
                    UiAction::SettingsSensors, UiAction::SettingsAdvanced};
                handleAction(actions[std::min<int32_t>(5, row)]);
            }
            break;
        case UiScreenId::SettingsWifi:
            handleAction(point.y < top_h ? UiAction::BackSettings
                                         : UiAction::WifiHotspot);
            break;
        case UiScreenId::SettingsDisplay:
            if (point.y < top_h) handleAction(UiAction::BackSettings);
            else if (point.y < height_ / 2)
                handleAction(left ? UiAction::BrightnessMinus
                                  : UiAction::BrightnessPlus);
            else
                handleAction(left ? UiAction::TimeoutMinus
                                  : UiAction::TimeoutPlus);
            break;
        case UiScreenId::SettingsUnits:
            if (point.y < top_h) handleAction(UiAction::BackSettings);
            else if (point.y < height_ / 2)
                handleAction(UiAction::TempUnitToggle);
            else
                handleAction(left ? UiAction::LanguageToggle
                                  : UiAction::HighContrastToggle);
            break;
        case UiScreenId::SettingsTouch:
            if (point.y < top_h) handleAction(UiAction::BackSettings);
            else
                handleAction(left ? UiAction::SensitivityCycle
                                  : UiAction::CalibrationStart);
            break;
        case UiScreenId::SettingsAdvanced:
            handleAction(point.y < top_h ? UiAction::BackSettings
                                         : UiAction::FactoryResetPrompt);
            break;
        case UiScreenId::SettingsSensors:
            handleAction(UiAction::BackSettings);
            break;
        case UiScreenId::DialogKeypad:
            if (point.y < top_h) handleAction(UiAction::Cancel);
            else if (lower && left) handleAction(UiAction::KeypadOk);
            else if (lower) handleAction(UiAction::KeypadBackspace);
            else handleAction(UiAction::KeypadDigit, 0);
            break;
        case UiScreenId::HistoryList:
            if (point.y < top_h) handleAction(UiAction::BackHome);
            else if (lower) handleAction(UiAction::HistoryMore);
            else handleAction(UiAction::HistorySelect, 0);
            break;
        case UiScreenId::HistoryDetail:
            if (point.y < top_h) handleAction(UiAction::BackHistory);
            break;
        default:
            show(UiScreenId::Home);
            break;
    }
}

void UiApp::handleAction(UiAction action, int32_t value) {
    playTouchFeedback();
    switch (action) {
        case UiAction::HomeStart:
            fallback_profile_index_ = 0;
            show(UiScreenId::StartPresets);
            break;
        case UiAction::HomeSettings:
            show(UiScreenId::SettingsMain);
            break;
        case UiAction::HomeHistory:
            history_offset_ = 0;
            show(UiScreenId::HistoryList);
            break;
        case UiAction::BackHome:
            show(UiScreenId::Home);
            break;
        case UiAction::BackSettings:
            show(UiScreenId::SettingsMain);
            break;
        case UiAction::BackPresets:
            show(UiScreenId::StartPresets);
            break;
        case UiAction::BackHistory:
            show(UiScreenId::HistoryList);
            break;
        case UiAction::PresetSelect: {
            const auto items = profiles();
            if (value < 0 || static_cast<size_t>(value) >= items.size()) break;
            pending_profile_id_ = items[static_cast<size_t>(value)].id;
            confirm_mode_ = ConfirmMode::StartProfile;
            show(UiScreenId::DialogConfirm);
            break;
        }
        case UiAction::PresetCustom:
            show(UiScreenId::StartCustom);
            break;
        case UiAction::CustomTempMinus:
        case UiAction::TargetTempMinus:
            draft_temp_c_ = std::max(30.0f, draft_temp_c_ - 1.0f);
            renderScreen();
            break;
        case UiAction::CustomTempPlus:
        case UiAction::TargetTempPlus:
            draft_temp_c_ = std::min(80.0f, draft_temp_c_ + 1.0f);
            renderScreen();
            break;
        case UiAction::CustomHumidityMinus:
        case UiAction::TargetHumidityMinus:
            draft_humidity_pct_ =
                std::max(5.0f, draft_humidity_pct_ - 1.0f);
            renderScreen();
            break;
        case UiAction::CustomHumidityPlus:
        case UiAction::TargetHumidityPlus:
            draft_humidity_pct_ =
                std::min(50.0f, draft_humidity_pct_ + 1.0f);
            renderScreen();
            break;
        case UiAction::CustomDurationMinus:
        case UiAction::TargetDurationMinus:
            draft_duration_min_ =
                draft_duration_min_ > 5 ? draft_duration_min_ - 5 : 1;
            renderScreen();
            break;
        case UiAction::CustomDurationPlus:
        case UiAction::TargetDurationPlus:
            draft_duration_min_ =
                std::min<uint16_t>(1440, draft_duration_min_ + 5);
            renderScreen();
            break;
        case UiAction::CustomStart:
            confirm_mode_ = ConfirmMode::StartCustom;
            show(UiScreenId::DialogConfirm);
            break;
        case UiAction::Confirm:
            if (confirm_mode_ == ConfirmMode::StartProfile) {
                if (controller_.startFromProfile(pending_profile_id_)) {
                    show(UiScreenId::Monitoring);
                } else {
                    showToast(tr("toast_start_fail"));
                    show(UiScreenId::StartPresets);
                }
            } else if (confirm_mode_ == ConfirmMode::StartCustom) {
                if (controller_.startCustom(draft_temp_c_, draft_humidity_pct_,
                                            draft_duration_min_)) {
                    show(UiScreenId::Monitoring);
                } else {
                    showToast(tr("toast_invalid"));
                    show(UiScreenId::StartCustom);
                }
            } else if (confirm_mode_ == ConfirmMode::StopCycle) {
                if (controller_.stop()) show(UiScreenId::Home);
            } else {
                controller_.config().factoryReset();
                settings_ = controller_.getUiSettings();
                language_ = settings_.language;
                if (display_manager_) {
                    display_manager_->setBrightness(
                        static_cast<uint8_t>(
                            (settings_.brightness_pct * 255u) / 100u));
                }
                showToast(tr("toast_reset"));
                show(UiScreenId::Home);
            }
            break;
        case UiAction::Cancel: {
            const UiScreenId back = previous_;
            show(back);
            break;
        }
        case UiAction::PauseResume: {
            const bool success =
                isPaused() ? controller_.resume() : controller_.pause();
            if (!success) {
                showToast(tr("toast_rejected"));
            }
            renderScreen();
            break;
        }
        case UiAction::StopPrompt:
            confirm_mode_ = ConfirmMode::StopCycle;
            show(UiScreenId::DialogConfirm);
            break;
        case UiAction::OpenTargetAdjust: {
            const DryingSession& active = session();
            draft_temp_c_ = active.target_temp_c;
            draft_humidity_pct_ = active.target_humidity_pct;
            draft_duration_min_ = active.max_duration_min;
            show(UiScreenId::TargetAdjust);
            break;
        }
        case UiAction::ApplyTargets:
            if (controller_.applyTargets(draft_temp_c_, draft_humidity_pct_,
                                         draft_duration_min_)) {
                show(UiScreenId::Monitoring);
            } else {
                showToast(tr("toast_target_rejected"));
                renderScreen();
            }
            break;
        case UiAction::SettingsWifi:
            show(UiScreenId::SettingsWifi);
            break;
        case UiAction::SettingsDisplay:
            show(UiScreenId::SettingsDisplay);
            break;
        case UiAction::SettingsUnits:
            show(UiScreenId::SettingsUnits);
            break;
        case UiAction::SettingsTouch:
            show(UiScreenId::SettingsTouch);
            break;
        case UiAction::SettingsAdvanced:
            show(UiScreenId::SettingsAdvanced);
            break;
        case UiAction::SettingsSensors:
            show(UiScreenId::SettingsSensors);
            break;
        case UiAction::WifiHotspot:
            controller_.requestWifiHotspot();
            showToast(tr("toast_hotspot"));
            renderScreen();
            break;
        case UiAction::BrightnessMinus:
            settings_.brightness_pct =
                settings_.brightness_pct > 20
                    ? settings_.brightness_pct - 10
                    : 10;
            saveUiSettings();
            break;
        case UiAction::BrightnessPlus:
            settings_.brightness_pct =
                std::min<uint8_t>(100, settings_.brightness_pct + 10);
            saveUiSettings();
            break;
        case UiAction::TimeoutMinus:
            settings_.timeout_sec =
                settings_.timeout_sec > 60 ? settings_.timeout_sec - 30 : 30;
            saveUiSettings();
            break;
        case UiAction::TimeoutPlus:
            settings_.timeout_sec =
                std::min<uint16_t>(600, settings_.timeout_sec + 30);
            saveUiSettings();
            break;
        case UiAction::OrientationNext:
            settings_.orientation =
                settings_.orientation == 270 ? 0 : settings_.orientation + 90;
            saveUiSettings();
            applyOrientation();
            break;
        case UiAction::TempUnitToggle:
            settings_.temp_unit =
                settings_.temp_unit == "celsius" ? "fahrenheit" : "celsius";
            saveUiSettings();
            break;
        case UiAction::LanguageToggle:
            settings_.language =
                settings_.language == "pt_br" ? "en_us" : "pt_br";
            saveUiSettings();
            break;
        case UiAction::HighContrastToggle:
            settings_.high_contrast = !settings_.high_contrast;
            saveUiSettings();
            break;
        case UiAction::SensitivityCycle: {
            if (settings_.touch_sensitivity == "low")
                settings_.touch_sensitivity = "medium";
            else if (settings_.touch_sensitivity == "medium")
                settings_.touch_sensitivity = "high";
            else
                settings_.touch_sensitivity = "low";
            TouchConfig touch_config = controller_.getTouchConfig();
            touch_config.sensitivity = settings_.touch_sensitivity;
            controller_.applyTouchConfig(touch_config);
            touch_.setSensitivity(touch_config.sensitivity);
            saveUiSettings();
            break;
        }
        case UiAction::CalibrationStart:
            startCalibration();
            break;
        case UiAction::FactoryResetPrompt:
            confirm_mode_ = ConfirmMode::FactoryReset;
            show(UiScreenId::DialogConfirm);
            break;
        case UiAction::HistoryMore:
            if (hasMoreHistory()) {
                history_offset_ += CycleHistoryStore::kInitialPage;
                renderScreen();
            }
            break;
        case UiAction::HistorySelect: {
            const auto records = historyPage();
            if (value >= 0 && static_cast<size_t>(value) < records.size()) {
                selected_history_id_ = records[static_cast<size_t>(value)].id;
                show(UiScreenId::HistoryDetail);
            }
            break;
        }
        case UiAction::HistoryExport:
            if (exportSelectedHistory()) {
                showToast(tr("toast_export_ok"));
            } else {
                showToast(tr("toast_export_fail"));
            }
            renderScreen();
            break;
        case UiAction::OpenKeypad:
            openKeypad(static_cast<KeypadField>(value));
            break;
        case UiAction::KeypadDigit:
            if (keypad_buffer_.length() < 5) {
                keypad_buffer_ += String(static_cast<char>('0' + constrain(value, 0, 9)));
            }
            renderScreen();
            break;
        case UiAction::KeypadBackspace:
            if (!keypad_buffer_.isEmpty()) {
                keypad_buffer_.remove(keypad_buffer_.length() - 1);
            }
            renderScreen();
            break;
        case UiAction::KeypadOk:
            applyKeypadValue();
            break;
        default:
            break;
    }
}

String UiApp::confirmationText() const {
    if (confirm_mode_ == ConfirmMode::StopCycle) {
        return tr("msg_stop");
    }
    if (confirm_mode_ == ConfirmMode::FactoryReset) {
        return tr("msg_reset");
    }
    if (confirm_mode_ == ConfirmMode::StartCustom) {
        return String(tr("title_custom")) + " " +
               formatTemp(draft_temp_c_, 0) + " / " +
               String(draft_humidity_pct_, 0) + "% / " +
               String(draft_duration_min_) + " min";
    }
    return String(tr("btn_confirm")) + " " + pending_profile_id_ + "?";
}

void UiApp::saveUiSettings() {
    if (!controller_.applyUiSettings(settings_)) {
        settings_ = controller_.getUiSettings();
        showToast(tr("toast_settings_rejected"));
    }
    language_ = settings_.language;
    if (display_manager_) {
        display_manager_->setBrightness(
            static_cast<uint8_t>((settings_.brightness_pct * 255u) / 100u));
    }
    dimmed_ = false;
    renderScreen();
}

void UiApp::updateInactivity() {
    if (!display_manager_) return;
    const uint32_t touch_ms =
        lvgl_ready_ ? std::max(last_touch_ms_, lvgl_.lastTouchMs())
                    : last_touch_ms_;
    if (touch_ms > last_touch_ms_) last_touch_ms_ = touch_ms;

    if (dimmed_ && millis() - last_touch_ms_ < 500) {
        display_manager_->setBrightness(
            static_cast<uint8_t>((settings_.brightness_pct * 255u) / 100u));
        dimmed_ = false;
        return;
    }
    if (!dimmed_ &&
        millis() - last_touch_ms_ >=
            static_cast<uint32_t>(settings_.timeout_sec) * 1000UL) {
        controller_.applyUiSettings(settings_);
        if (ui_format::isActiveCycle(controller_.getState()) &&
            current_ != UiScreenId::Monitoring &&
            current_ != UiScreenId::DialogConfirm) {
            show(UiScreenId::Monitoring);
        }
        display_manager_->setBrightness(25);
        dimmed_ = true;
    }
}

void UiApp::showToast(const String& text, uint32_t duration_ms) {
    toast_text_ = text;
    toast_until_ms_ = millis() + duration_ms;
}

void UiApp::playTouchFeedback() {
    if (haptic_callback_) {
        haptic_callback_();
    }
}

void UiApp::applyTouchCalibrationToManager(const TouchConfig& config) {
    TouchManager::Calibration calibration;
    calibration.x_min = config.calibration.x_min;
    calibration.x_max = config.calibration.x_max;
    calibration.y_min = config.calibration.y_min;
    calibration.y_max = config.calibration.y_max;
    calibration.width = width_;
    calibration.height = height_;
    calibration.swap_xy =
        config.swap_xy || config.calibration.swapped_xy;
    calibration.invert_x = config.invert_x;
    calibration.invert_y = config.invert_y;
    touch_.setCalibration(calibration);
}

void UiApp::startCalibration() {
    calibration_active_ = true;
    calibration_touch_down_ = false;
    calibration_point_index_ = 0;
    touch_.setCalibration(TouchManager::Calibration{});
    renderScreen();
}

void UiApp::handleCalibrationTouch() {
    const TouchPoint point = touch_.read();
    if (!point.pressed) {
        calibration_touch_down_ = false;
        return;
    }
    last_touch_ms_ = point.timestamp_ms ? point.timestamp_ms : millis();
    if (calibration_touch_down_) return;
    calibration_touch_down_ = true;
    calibration_points_[calibration_point_index_++] = point;
    if (calibration_point_index_ >= 4) finishCalibration();
    else renderScreen();
}

void UiApp::finishCalibration() {
    TouchConfig config = controller_.getTouchConfig();
    config.calibration.x_min = static_cast<int16_t>(
        (static_cast<int32_t>(calibration_points_[0].x) +
         calibration_points_[3].x) /
        2);
    config.calibration.x_max = static_cast<int16_t>(
        (static_cast<int32_t>(calibration_points_[1].x) +
         calibration_points_[2].x) /
        2);
    config.calibration.y_min = static_cast<int16_t>(
        (static_cast<int32_t>(calibration_points_[0].y) +
         calibration_points_[1].y) /
        2);
    config.calibration.y_max = static_cast<int16_t>(
        (static_cast<int32_t>(calibration_points_[2].y) +
         calibration_points_[3].y) /
        2);
    calibration_active_ = false;
    calibration_point_index_ = 0;
    if (controller_.applyTouchConfig(config)) {
        applyTouchCalibrationToManager(config);
        showToast(tr("toast_cal_ok"));
    } else {
        applyTouchCalibrationToManager(controller_.getTouchConfig());
        showToast(tr("toast_cal_fail"));
    }
    renderScreen();
}

void UiApp::applyOrientation() {
    if (!display_manager_) return;
    display_manager_->setRotation(
        ui_format::orientationToLgfx(settings_.orientation));
    const bool swapped =
        settings_.orientation == 90 || settings_.orientation == 270;
    width_ = swapped ? native_height_ : native_width_;
    height_ = swapped ? native_width_ : native_height_;
    if (lvgl_ready_) {
        lvgl_.deinit();
        lvgl_ready_ = lvgl_.init(touch_, width_, height_, flushToDisplay,
                                 display_manager_);
        renderScreen();
    }
}

void UiApp::syncFromSharedState() {
    const SystemState state = controller_.getState();
    if (ui_format::isActiveCycle(state) &&
        (current_ == UiScreenId::Home || current_ == UiScreenId::Splash)) {
        show(UiScreenId::Monitoring);
    }
}

void UiApp::openKeypad(KeypadField field) {
    keypad_field_ = field;
    if (field == KeypadField::Temperature) {
        keypad_buffer_ = String(static_cast<int>(draft_temp_c_));
    } else if (field == KeypadField::Humidity) {
        keypad_buffer_ = String(static_cast<int>(draft_humidity_pct_));
    } else {
        keypad_buffer_ = String(draft_duration_min_);
    }
    show(UiScreenId::DialogKeypad);
}

void UiApp::applyKeypadValue() {
    const int value = keypad_buffer_.toInt();
    if (keypad_field_ == KeypadField::Temperature) {
        draft_temp_c_ = constrain(static_cast<float>(value), 30.0f, 80.0f);
    } else if (keypad_field_ == KeypadField::Humidity) {
        draft_humidity_pct_ =
            constrain(static_cast<float>(value), 5.0f, 50.0f);
    } else {
        draft_duration_min_ =
            static_cast<uint16_t>(constrain(value, 1, 1440));
    }
    show(previous_ == UiScreenId::StartCustom ? UiScreenId::StartCustom
                                              : UiScreenId::TargetAdjust);
}

bool UiApp::exportSelectedHistory() {
    CycleRecord record;
    if (!selectedHistory(record)) {
        return false;
    }
    JsonDocument doc;
    if (!history_.fillExportJson(record, doc.to<JsonObject>())) {
        return false;
    }
    controller_.broadcastJson(doc.as<JsonObject>());
    history_.writeCsvToSd(record);
    return true;
}

void UiApp::flushToDisplay(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                           const uint16_t* pixels, void* context) {
    DisplayManager* manager = static_cast<DisplayManager*>(context);
    if (!manager || !pixels || x2 < x1 || y2 < y1) return;
    manager->pushRgb565(x1, y1, static_cast<uint16_t>(x2 - x1 + 1),
                        static_cast<uint16_t>(y2 - y1 + 1), pixels);
}

void UiApp::renderScreen() {
#if !defined(UNIT_TEST)
    if (!lvgl_ready_) return;
    lv_obj_t* next = nullptr;
    switch (current_) {
        case UiScreenId::Home:
            next = ui_screens::buildHome(*this);
            break;
        case UiScreenId::StartPresets:
            next = ui_screens::buildStartPresets(*this);
            break;
        case UiScreenId::StartCustom:
            next = ui_screens::buildStartCustom(*this);
            break;
        case UiScreenId::Monitoring:
            next = ui_screens::buildMonitoring(*this);
            break;
        case UiScreenId::DialogConfirm:
            next = ui_screens::buildDialogConfirm(*this);
            break;
        case UiScreenId::TargetAdjust:
            next = ui_screens::buildTargetAdjust(*this);
            break;
        case UiScreenId::SettingsMain:
            next = ui_screens::buildSettingsMain(*this);
            break;
        case UiScreenId::SettingsWifi:
            next = ui_screens::buildSettingsWifi(*this);
            break;
        case UiScreenId::SettingsDisplay:
            next = ui_screens::buildSettingsDisplay(*this);
            break;
        case UiScreenId::SettingsUnits:
            next = ui_screens::buildSettingsUnits(*this);
            break;
        case UiScreenId::SettingsTouch:
            next = ui_screens::buildSettingsTouch(*this);
            break;
        case UiScreenId::SettingsAdvanced:
            next = ui_screens::buildSettingsAdvanced(*this);
            break;
        case UiScreenId::SettingsSensors:
            next = ui_screens::buildSettingsSensors(*this);
            break;
        case UiScreenId::DialogKeypad:
            next = ui_screens::buildDialogKeypad(*this);
            break;
        case UiScreenId::HistoryList:
            next = ui_screens::buildHistoryList(*this);
            break;
        case UiScreenId::HistoryDetail:
            next = ui_screens::buildHistoryDetail(*this);
            break;
        default:
            current_ = UiScreenId::Home;
            next = ui_screens::buildHome(*this);
            break;
    }
    if (!next) return;
    if (!toast_text_.isEmpty() &&
        static_cast<int32_t>(toast_until_ms_ - millis()) > 0) {
        lv_obj_t* toast = lv_label_create(next);
        lv_label_set_text(toast, toast_text_.c_str());
        lv_obj_set_style_bg_opa(toast, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(toast, lv_color_hex(ui_theme::warning), 0);
        lv_obj_set_style_text_color(toast, lv_color_hex(0x000000), 0);
        lv_obj_set_style_pad_all(toast, 8, 0);
        lv_obj_align(toast, LV_ALIGN_TOP_MID, 0, 4);
    }
    lv_scr_load_anim(next, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
    root_ = next;
#endif
}

#if !defined(UNIT_TEST)
lv_obj_t* UiApp::createScreen() {
    action_binding_count_ = 0;
    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(screen, 8, 0);
    lv_obj_set_style_pad_row(screen, 6, 0);
    lv_obj_set_style_bg_color(
        screen, lv_color_hex(ui_theme::backgroundFor(highContrast())), 0);
    lv_obj_set_style_text_color(
        screen, lv_color_hex(ui_theme::textFor(highContrast())), 0);
    return screen;
}

lv_obj_t* UiApp::addTitle(lv_obj_t* parent, const String& text) {
    lv_obj_t* label = addLabel(parent, text);
    lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
    return label;
}

lv_obj_t* UiApp::addLabel(lv_obj_t* parent, const String& text) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text.c_str());
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, LV_PCT(100));
    lv_obj_set_style_text_color(
        label, lv_color_hex(ui_theme::textFor(highContrast())), 0);
    return label;
}

lv_obj_t* UiApp::addButton(lv_obj_t* parent, const String& text,
                           UiAction action, int32_t value) {
    if (action_binding_count_ >= kMaxActionBindings) return nullptr;
    ActionBinding& binding = action_bindings_[action_binding_count_++];
    binding.app = this;
    binding.action = action;
    binding.value = value;

    lv_obj_t* button = lv_btn_create(parent);
    lv_obj_set_width(button, LV_PCT(100));
    lv_obj_set_height(button, ui_theme::kMinTouchTargetPx);
    lv_obj_set_style_bg_color(
        button, lv_color_hex(ui_theme::surfaceFor(highContrast())), 0);
    lv_obj_set_style_border_width(button, highContrast() ? 2 : 1, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(ui_theme::border), 0);
    lv_obj_add_event_cb(button, lvActionCallback, LV_EVENT_CLICKED, &binding);
    lv_obj_t* label = lv_label_create(button);
    lv_label_set_text(label, text.c_str());
    lv_obj_set_style_text_color(
        label, lv_color_hex(ui_theme::textFor(highContrast())), 0);
    lv_obj_center(label);
    return button;
}

void UiApp::lvActionCallback(lv_event_t* event) {
    ActionBinding* binding =
        static_cast<ActionBinding*>(lv_event_get_user_data(event));
    if (binding && binding->app) {
        binding->app->last_touch_ms_ = millis();
        binding->app->handleAction(binding->action, binding->value);
    }
}
#endif

} // namespace filament_dryer
