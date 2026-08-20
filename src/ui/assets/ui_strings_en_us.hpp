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

#pragma once

#include <stddef.h>

namespace filament_dryer {
namespace ui_strings_en_us {

struct Entry {
    const char* key;
    const char* value;
};

inline constexpr Entry table[] = {
    {"btn_start", "Start Drying"},
    {"btn_stop", "Stop"},
    {"btn_pause", "Pause"},
    {"btn_resume", "Resume"},
    {"btn_confirm", "Confirm"},
    {"btn_cancel", "Cancel"},
    {"btn_more", "More"},
    {"btn_back", "Back"},
    {"btn_apply", "Apply"},
    {"btn_custom", "Custom"},
    {"btn_adjust", "Adjust targets"},
    {"btn_history", "History"},
    {"btn_settings", "Settings"},
    {"btn_reconfigure", "Reconfigure"},
    {"btn_calibrate", "Calibrate (4 points)"},
    {"btn_reset", "Reset settings"},
    {"btn_export", "Send to App"},
    {"btn_keypad", "Keypad"},
    {"title_presets", "Filament profiles"},
    {"title_history", "History"},
    {"title_settings", "Settings"},
    {"title_custom", "Custom cycle"},
    {"title_drying", "Drying"},
    {"title_paused", "Paused"},
    {"title_wifi", "WiFi"},
    {"title_display", "Display"},
    {"title_units", "Units and language"},
    {"title_touch", "Touch"},
    {"title_advanced", "Advanced"},
    {"title_sensors", "Sensors"},
    {"title_keypad", "Numeric keypad"},
    {"title_confirm", "Confirm"},
    {"title_adjust", "Adjust targets"},
    {"status_idle", "idle"},
    {"status_drying", "drying"},
    {"status_paused", "paused"},
    {"status_stopped", "stopped"},
    {"label_temp", "Temp"},
    {"label_rh", "RH"},
    {"label_elapsed", "Elapsed"},
    {"label_remaining", "Remaining"},
    {"label_duration", "Duration"},
    {"label_brightness", "Brightness"},
    {"label_timeout", "Timeout"},
    {"label_orientation", "Orientation"},
    {"label_language", "Language"},
    {"label_contrast", "High contrast"},
    {"label_sensitivity", "Sensitivity"},
    {"label_ssid", "SSID"},
    {"label_rssi", "Signal"},
    {"label_firmware", "Firmware"},
    {"label_device", "Device"},
    {"label_safety", "Safety limit"},
    {"label_no_history", "No completed cycles"},
    {"label_not_found", "Record not found"},
    {"label_reason", "Reason"},
    {"label_average", "Average"},
    {"label_max", "Max"},
    {"label_target", "Target"},
    {"label_status", "Status"},
    {"sensors_readonly", "Sensors are configured by the installer."},
    {"wifi_hotspot_hint", "Start the configuration hotspot to change network."},
    {"msg_stop", "Stop the active cycle?"},
    {"msg_reset", "Reset all settings?"},
    {"toast_pause_timeout", "Pause timeout: cycle stopped"},
    {"toast_remote", "State changed remotely"},
    {"toast_invalid", "Invalid or unsafe targets"},
    {"toast_start_fail", "Unable to start cycle"},
    {"toast_rejected", "Command rejected"},
    {"toast_target_rejected", "Target update rejected"},
    {"toast_hotspot", "Configuration hotspot requested"},
    {"toast_settings_rejected", "Settings rejected"},
    {"toast_reset", "Settings reset"},
    {"toast_cal_ok", "Calibration saved"},
    {"toast_cal_fail", "Calibration failed"},
    {"toast_export_ok", "History sent"},
    {"toast_export_fail", "Export failed"},
    {"toast_timeout_saved", "Settings saved"},
    {"cal_top_left", "top left"},
    {"cal_top_right", "top right"},
    {"cal_bottom_right", "bottom right"},
    {"cal_bottom_left", "bottom left"},
    {"cal_prompt", "Touch target: "},
};
inline constexpr size_t table_size = sizeof(table) / sizeof(table[0]);

}  // namespace ui_strings_en_us
}  // namespace filament_dryer
