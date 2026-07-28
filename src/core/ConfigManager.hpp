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
 * ConfigManager - NVS and JSON Configuration Manager
 * Klipper-style object configuration with NVS persistence
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <string>
#include <vector>

namespace filament_dryer {

// Structs at namespace scope (not inside class)
struct PidConfig {
    float kp = 0.0f;
    float ki = 0.0f;
    float kd = 0.0f;
    bool calibrated = false;
    
    PidConfig() = default;
    PidConfig(float p, float i, float d) : kp(p), ki(i), kd(d), calibrated(true) {}
};

struct SensorConfig {
    String type = "sht31";          // primary / temperature sensor type
    bool is_integrated = true;      // true if temp+humidity in same IC
    uint8_t i2c_bus = 0;
    uint8_t i2c_address = 0x44;
    int8_t gpio_pin = -1;
    int8_t sda_pin = 21;
    int8_t scl_pin = 22;
    float temperature_offset = 0.0f;
    float temperature_scale = 1.0f;
    float humidity_offset = 0.0f;
    float humidity_scale = 1.0f;
    // Separate humidity sensor (when is_integrated == false)
    String humidity_type;
    uint8_t humidity_i2c_address = 0;
    int8_t humidity_gpio_pin = -1;
    int8_t humidity_sda_pin = 21;
    int8_t humidity_scl_pin = 22;
    // Optional extra temperature sensor id/type for status reflection
    String extra_temp_type;
    int8_t extra_temp_gpio_pin = -1;
    uint8_t extra_temp_i2c_address = 0;
};

struct FanCurvePoint {
    float temp_c = 0.0f;
    float power_pct = 80.0f;
};

struct ActuatorConfig {
    String heater_type = "mosfet_pwm";
    int8_t heater_pin = 25;
    uint32_t heater_pwm_freq = 1000;
    uint8_t heater_max_power_pct = 100;
    float heater_max_temp_c = 0.0f;  // 0 = unset; from safety_limits.max_temp_c
    String fan_type = "fan_pwm";
    String fan_mode = "independent_pwm";
    int8_t fan_pin = 26;
    uint32_t fan_pwm_freq = 5000;
    float fan_duty_pct = 80.0f;
    std::vector<FanCurvePoint> fan_speed_curve;
    uint16_t cooldown_duration_sec = 30;
    bool has_custom = false;
    String custom_type;
    int8_t custom_pin = -1;
};

struct DisplayConfig {
    bool enabled = false;
    String driver = "auto";           // ssd1306, sh1106, st7789, ili9341, st7735, gc9a01, ili9488, hd44780, nextion, auto
    String bus_type = "i2c";          // i2c, spi, uart (parallel_8bit rejected at parse)
    uint16_t width = 128;
    uint16_t height = 64;
    uint16_t rotation = 0;            // 0, 90, 180, 270
    int8_t spi_mosi = -1;
    int8_t spi_sclk = -1;
    int8_t spi_cs = -1;
    int8_t dc_pin = -1;
    int8_t rst_pin = -1;
    int8_t backlight_pin = -1;
    int8_t i2c_sda = 21;
    int8_t i2c_scl = 22;
    uint8_t i2c_address = 0x3C;
    uint8_t refresh_rate_hz = 1;          // Display refresh 1–5 Hz
    String font_scaling = "auto";         // auto|small|medium|large
    bool compact_mode = false;
    std::vector<String> fields = {"chamber_temp_c", "target_temp_c", "humidity_pct", "heater_power_pct", "status"};
};

struct FilamentProfile {
    String id;
    String name_pt;
    String name_en;
    float target_temp_c = 50.0f;
    uint16_t default_duration_min = 240;
    float target_humidity_pct = 15.0f;
    bool is_builtin = true;
    uint32_t created_at = 0;
    uint32_t updated_at = 0;
};

struct WifiConfig {
    String ssid;
    String password;
    bool valid = false;
};

struct ControlConfig {
    String algorithm = "pid";              // pid, bang_bang, pwm_feedforward, custom
    bool auto_tune = false;
    JsonObject parameters;                  // Algorithm-specific parameters
    struct SafetyLimits {
        float hard_temp_limit_c = 80.0f;
        int max_heater_power_pct = 100;
        int sensor_timeout_ms = 600;
        int thermal_runaway_time_sec = 45;
        float thermal_runaway_temp_rise_c = 0.5f;
    } safety_limits;
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    bool begin();
    bool load();
    bool save();
    
    // WiFi Configuration
    WifiConfig getWifiConfig() const;
    bool setWifiConfig(const WifiConfig& config);
    
    // Sensor Configuration
    SensorConfig getSensorConfig() const;
    void setSensorConfig(const SensorConfig& config);
    
    // Actuator Configuration
    ActuatorConfig getActuatorConfig() const;
    void setActuatorConfig(const ActuatorConfig& config);
    
    // Display Configuration
    DisplayConfig getDisplayConfig() const;
    void setDisplayConfig(const DisplayConfig& config);
    
    // PID Configuration
    PidConfig getPidConfig() const;
    void setPidConfig(const PidConfig& config);
    
    // Filament Profiles
    std::vector<FilamentProfile> getProfiles() const;
    FilamentProfile getProfile(const String& profile_id) const;
    bool addProfile(const FilamentProfile& profile);
    bool updateProfile(const FilamentProfile& profile);
    bool deleteProfile(const String& profile_id);
    void resetProfilesToDefaults();
    static bool validateProfileParams(float temp_c, uint16_t duration_min, float humidity_pct) {
        if (temp_c < 30.0f || temp_c > 80.0f) return false;
        if (duration_min < 1 || duration_min > 1440) return false;
        if (humidity_pct < 5.0f || humidity_pct > 50.0f) return false;
        return true;
    }
    static bool validateStartParams(float temp_c, uint16_t duration_min, float humidity_pct,
                                    bool has_humidity) {
        if (temp_c < 30.0f || temp_c > 80.0f) return false;
        if (duration_min < 1 || duration_min > 1440) return false;
        if (has_humidity && (humidity_pct < 5.0f || humidity_pct > 50.0f)) return false;
        return true;
    }
    static constexpr size_t kMaxCustomProfiles = 20;
    
    // Generic JSON config (Klipper-style object config)
    bool setObjectConfig(const String& object_name, const JsonObject& config);
    JsonObject getObjectConfig(const String& object_name);
    std::vector<String> listObjectConfigs() const;
    
    // Factory reset
    void factoryReset();
    
    // JSON serialization
    String toJson() const;
    bool fromJson(const String& json);
    
    // Access preferences for direct NVS operations
    Preferences& getPreferences() { return prefs_; }

private:
    mutable Preferences prefs_;
    bool initialized_ = false;
    
    // NVS Keys
    static constexpr const char* NVS_NAMESPACE = "filament_dryer";
    static constexpr const char* KEY_WIFI = "wifi";
    static constexpr const char* KEY_SENSOR = "sensor";
    static constexpr const char* KEY_ACTUATOR = "actuator";
    static constexpr const char* KEY_DISPLAY = "display";
    static constexpr const char* KEY_PID = "pid";
    static constexpr const char* KEY_PROFILES = "profiles";
    static constexpr const char* KEY_OBJECTS = "objects";
    
    // Default profiles
    static std::vector<FilamentProfile> getDefaultProfiles();
    
    // Helper methods
    template<typename T>
    bool readStruct(const char* key, T& value);
    template<typename T>
    bool writeStruct(const char* key, const T& value);
    bool readString(const char* key, String& value);
    bool writeString(const char* key, const String& value);
    bool readJsonArray(const char* key, std::vector<String>& array) const;
    bool writeJsonArray(const char* key, const std::vector<String>& array);
    
    // Private helper for profile operations
    bool saveProfiles(const std::vector<FilamentProfile>& profiles);
};

} // namespace filament_dryer