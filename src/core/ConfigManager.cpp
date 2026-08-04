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
 * ConfigManager - Implementation
 * NVS and JSON Configuration Manager
 * Compatible with ArduinoJson v7
 */
#include "ConfigManager.hpp"
#include "firmware_version.h"
#include <Arduino.h>

namespace filament_dryer {

ConfigManager::ConfigManager() {}

ConfigManager::~ConfigManager() {
    if (initialized_) {
        prefs_.end();
    }
}

bool ConfigManager::begin() {
    if (initialized_) return true;
    
    initialized_ = prefs_.begin(NVS_NAMESPACE, false);
    if (!initialized_) {
        return false;
    }
    
    return load();
}

bool ConfigManager::load() {
    if (!initialized_) return false;
    
    // Try to load each config section
    WifiConfig wifi;
    if (readStruct(KEY_WIFI, wifi)) {
        // WiFi config loaded
    }
    
    return true;
}

bool ConfigManager::save() {
    if (!initialized_) return false;
    return true; // Preferences auto-commit on put
}

// WiFi Configuration
WifiConfig ConfigManager::getWifiConfig() const {
    WifiConfig config;
    if (!initialized_) return config;
    
    String json;
    if (prefs_.getString(KEY_WIFI, json)) {
        JsonDocument doc;
        if (deserializeJson(doc, json) == DeserializationError::Ok) {
            config.ssid = doc["ssid"] | "";
            config.password = doc["password"] | "";
            config.valid = !config.ssid.isEmpty();
            // Flat fields (installer NVS) or nested static_ip object
            if (doc["static_ip"].is<JsonObject>()) {
                JsonObject sip = doc["static_ip"].as<JsonObject>();
                config.ip = sip["ip"] | "";
                config.gateway = sip["gateway"] | "";
                config.netmask = sip["netmask"] | "255.255.255.0";
                config.dns = sip["dns"] | "";
            } else {
                config.ip = doc["ip"] | "";
                config.gateway = doc["gateway"] | "";
                config.netmask = doc["netmask"] | "255.255.255.0";
                config.dns = doc["dns"] | "";
            }
            config.use_static_ip = doc["use_static_ip"] | !config.ip.isEmpty();
        }
    }
    return config;
}

bool ConfigManager::setWifiConfig(const WifiConfig& config) {
    if (!initialized_) return false;
    
    JsonDocument doc;
    doc["ssid"] = config.ssid;
    doc["password"] = config.password;
    doc["use_static_ip"] = config.use_static_ip && !config.ip.isEmpty();
    if (!config.ip.isEmpty()) {
        doc["ip"] = config.ip;
        doc["gateway"] = config.gateway;
        doc["netmask"] = config.netmask.isEmpty() ? "255.255.255.0" : config.netmask;
        doc["dns"] = config.dns;
        JsonObject sip = doc["static_ip"].to<JsonObject>();
        sip["ip"] = config.ip;
        sip["gateway"] = config.gateway;
        sip["netmask"] = config.netmask.isEmpty() ? "255.255.255.0" : config.netmask;
        sip["dns"] = config.dns;
    }
    
    String json;
    serializeJson(doc, json);
    return writeString(KEY_WIFI, json);
}

// Sensor Configuration
SensorConfig ConfigManager::getSensorConfig() const {
    SensorConfig config;
    if (!initialized_) return config;
    
    String json;
    if (prefs_.getString(KEY_SENSOR, json)) {
        JsonDocument doc;
        if (deserializeJson(doc, json) == DeserializationError::Ok) {
            config.type = doc["type"] | "sht31";
            config.is_integrated = doc["is_integrated"] | true;
            config.i2c_bus = doc["i2c_bus"] | 0;
            config.i2c_address = doc["i2c_address"] | 0x44;
            config.gpio_pin = doc["gpio_pin"] | -1;
            config.sda_pin = doc["sda_pin"] | 21;
            config.scl_pin = doc["scl_pin"] | 22;
            config.temperature_offset = doc["temperature_offset"] | 0.0f;
            config.temperature_scale = doc["temperature_scale"] | 1.0f;
            config.humidity_offset = doc["humidity_offset"] | 0.0f;
            config.humidity_scale = doc["humidity_scale"] | 1.0f;
            config.is_integrated = doc["is_integrated"] | true;
            config.humidity_type = doc["humidity_type"] | "";
            config.humidity_i2c_address = doc["humidity_i2c_address"] | 0;
            config.humidity_gpio_pin = doc["humidity_gpio_pin"] | -1;
            config.humidity_sda_pin = doc["humidity_sda_pin"] | 21;
            config.humidity_scl_pin = doc["humidity_scl_pin"] | 22;
            config.extra_temp_type = doc["extra_temp_type"] | "";
            config.extra_temp_gpio_pin = doc["extra_temp_gpio_pin"] | -1;
            config.extra_temp_i2c_address = doc["extra_temp_i2c_address"] | 0;
        }
    }
    return config;
}

void ConfigManager::setSensorConfig(const SensorConfig& config) {
    if (!initialized_) return;
    
    JsonDocument doc;
    doc["type"] = config.type;
    doc["is_integrated"] = config.is_integrated;
    doc["i2c_bus"] = config.i2c_bus;
    doc["i2c_address"] = config.i2c_address;
    doc["gpio_pin"] = config.gpio_pin;
    doc["sda_pin"] = config.sda_pin;
    doc["scl_pin"] = config.scl_pin;
    doc["temperature_offset"] = config.temperature_offset;
    doc["temperature_scale"] = config.temperature_scale;
    doc["humidity_offset"] = config.humidity_offset;
    doc["humidity_scale"] = config.humidity_scale;
    doc["humidity_type"] = config.humidity_type;
    doc["humidity_i2c_address"] = config.humidity_i2c_address;
    doc["humidity_gpio_pin"] = config.humidity_gpio_pin;
    doc["humidity_sda_pin"] = config.humidity_sda_pin;
    doc["humidity_scl_pin"] = config.humidity_scl_pin;
    doc["extra_temp_type"] = config.extra_temp_type;
    doc["extra_temp_gpio_pin"] = config.extra_temp_gpio_pin;
    doc["extra_temp_i2c_address"] = config.extra_temp_i2c_address;
    
    String json;
    serializeJson(doc, json);
    writeString(KEY_SENSOR, json);
}

// Actuator Configuration
ActuatorConfig ConfigManager::getActuatorConfig() const {
    ActuatorConfig config;
    if (!initialized_) return config;
    
    String json;
    if (prefs_.getString(KEY_ACTUATOR, json)) {
        JsonDocument doc;
        if (deserializeJson(doc, json) == DeserializationError::Ok) {
            config.heater_type = doc["heater_type"] | "mosfet_pwm";
            config.heater_pin = doc["heater_pin"] | 25;
            config.heater_pwm_freq = doc["heater_pwm_freq"] | 1000;
            config.heater_max_power_pct = doc["heater_max_power_pct"] | 100;
            config.heater_max_temp_c = doc["heater_max_temp_c"] | 0.0f;
            config.fan_type = doc["fan_type"] | "fan_pwm";
            config.fan_mode = doc["fan_mode"] | "independent_pwm";
            config.fan_pin = doc["fan_pin"] | 26;
            config.fan_pwm_freq = doc["fan_pwm_freq"] | 5000;
            config.fan_duty_pct = doc["fan_duty_pct"] | 80.0f;
            config.cooldown_duration_sec = doc["cooldown_duration_sec"] | 30;
            config.has_custom = doc["has_custom"] | false;
            config.custom_type = doc["custom_type"] | "";
            config.custom_pin = doc["custom_pin"] | -1;
            config.fan_speed_curve.clear();
            if (doc["fan_speed_curve"].is<JsonArray>()) {
                for (JsonVariant v : doc["fan_speed_curve"].as<JsonArray>()) {
                    FanCurvePoint pt;
                    if (v.is<JsonObject>()) {
                        pt.temp_c = v["temp_c"] | 0.0f;
                        pt.power_pct = v["power_pct"] | 80.0f;
                    } else {
                        pt.power_pct = v.as<float>();
                    }
                    config.fan_speed_curve.push_back(pt);
                }
            }
        }
    }
    return config;
}

void ConfigManager::setActuatorConfig(const ActuatorConfig& config) {
    if (!initialized_) return;
    
    JsonDocument doc;
    doc["heater_type"] = config.heater_type;
    doc["heater_pin"] = config.heater_pin;
    doc["heater_pwm_freq"] = config.heater_pwm_freq;
    doc["heater_max_power_pct"] = config.heater_max_power_pct;
    doc["heater_max_temp_c"] = config.heater_max_temp_c;
    doc["fan_type"] = config.fan_type;
    doc["fan_mode"] = config.fan_mode;
    doc["fan_pin"] = config.fan_pin;
    doc["fan_pwm_freq"] = config.fan_pwm_freq;
    doc["fan_duty_pct"] = config.fan_duty_pct;
    doc["cooldown_duration_sec"] = config.cooldown_duration_sec;
    doc["has_custom"] = config.has_custom;
    doc["custom_type"] = config.custom_type;
    doc["custom_pin"] = config.custom_pin;
    JsonArray curve = doc["fan_speed_curve"].to<JsonArray>();
    for (const auto& pt : config.fan_speed_curve) {
        JsonObject obj = curve.add<JsonObject>();
        obj["temp_c"] = pt.temp_c;
        obj["power_pct"] = pt.power_pct;
    }
    
    String json;
    serializeJson(doc, json);
    writeString(KEY_ACTUATOR, json);
}

// Display Configuration
DisplayConfig ConfigManager::getDisplayConfig() const {
    DisplayConfig config;
    if (!initialized_) return config;
    
    String json;
    if (prefs_.getString(KEY_DISPLAY, json)) {
        JsonDocument doc;
        if (deserializeJson(doc, json) == DeserializationError::Ok) {
            config.enabled = doc["enabled"] | false;
            config.driver = doc["driver"] | "auto";
            config.bus_type = doc["bus_type"] | "i2c";
            config.width = doc["width"] | 128;
            config.height = doc["height"] | 64;
            config.rotation = doc["rotation"] | 0;
            config.spi_mosi = doc["spi_mosi"] | -1;
            config.spi_sclk = doc["spi_sclk"] | -1;
            config.spi_cs = doc["spi_cs"] | -1;
            config.dc_pin = doc["dc_pin"] | -1;
            config.rst_pin = doc["rst_pin"] | -1;
            config.backlight_pin = doc["backlight_pin"] | -1;
            config.i2c_sda = doc["i2c_sda"] | 21;
            config.i2c_scl = doc["i2c_scl"] | 22;
            config.i2c_address = doc["i2c_address"] | 0x3C;
            config.refresh_rate_hz = doc["refresh_rate_hz"] | 1;
            if (config.refresh_rate_hz < 1) config.refresh_rate_hz = 1;
            if (config.refresh_rate_hz > 5) config.refresh_rate_hz = 5;
            config.font_scaling = doc["font_scaling"] | "auto";
            config.compact_mode = doc["compact_mode"] | false;
            
            JsonArray fields = doc["fields"];
            config.fields.clear();
            for (JsonVariant v : fields) {
                config.fields.push_back(v.as<String>());
            }
        }
    }
    return config;
}

void ConfigManager::setDisplayConfig(const DisplayConfig& config) {
    if (!initialized_) return;
    
    JsonDocument doc;
    doc["enabled"] = config.enabled;
    doc["driver"] = config.driver;
    doc["bus_type"] = config.bus_type;
    doc["width"] = config.width;
    doc["height"] = config.height;
    doc["rotation"] = config.rotation;
    doc["spi_mosi"] = config.spi_mosi;
    doc["spi_sclk"] = config.spi_sclk;
    doc["spi_cs"] = config.spi_cs;
    doc["dc_pin"] = config.dc_pin;
    doc["rst_pin"] = config.rst_pin;
    doc["backlight_pin"] = config.backlight_pin;
    doc["i2c_sda"] = config.i2c_sda;
    doc["i2c_scl"] = config.i2c_scl;
    doc["i2c_address"] = config.i2c_address;
    doc["refresh_rate_hz"] = config.refresh_rate_hz;
    doc["font_scaling"] = config.font_scaling;
    doc["compact_mode"] = config.compact_mode;
    
    JsonArray fields = doc["fields"].to<JsonArray>();
    for (const String& field : config.fields) {
        fields.add(field);
    }
    
    String json;
    serializeJson(doc, json);
    writeString(KEY_DISPLAY, json);
}

// PID Configuration
PidConfig ConfigManager::getPidConfig() const {
    PidConfig config;
    if (!initialized_) return config;
    
    String json;
    if (prefs_.getString(KEY_PID, json)) {
        JsonDocument doc;
        if (deserializeJson(doc, json) == DeserializationError::Ok) {
            config.kp = doc["kp"] | 0.0f;
            config.ki = doc["ki"] | 0.0f;
            config.kd = doc["kd"] | 0.0f;
            config.calibrated = doc["calibrated"] | false;
        }
    }
    return config;
}

bool ConfigManager::setPidConfig(const PidConfig& config) {
    if (!initialized_) return false;
    
    JsonDocument doc;
    doc["kp"] = config.kp;
    doc["ki"] = config.ki;
    doc["kd"] = config.kd;
    doc["calibrated"] = config.calibrated;
    
    String json;
    serializeJson(doc, json);
    return writeString(KEY_PID, json);
}

// Filament Profiles
std::vector<FilamentProfile> ConfigManager::getProfiles() const {
    std::vector<FilamentProfile> profiles;
    if (!initialized_) return profiles;
    
    String json;
    if (prefs_.getString(KEY_PROFILES, json)) {
        JsonDocument doc;
        if (deserializeJson(doc, json) == DeserializationError::Ok) {
            JsonArray arr = doc.as<JsonArray>();
            for (JsonVariant v : arr) {
                JsonObject obj = v.as<JsonObject>();
                FilamentProfile p;
                p.id = obj["id"] | "";
                p.name_pt = obj["name_pt"] | "";
                p.name_en = obj["name_en"] | "";
                p.target_temp_c = obj["target_temp_c"] | 50.0f;
                p.default_duration_min = obj["default_duration_min"] | 240;
                p.target_humidity_pct = obj["target_humidity_pct"] | 15.0f;
                p.is_builtin = obj["is_builtin"] | true;
                p.created_at = obj["created_at"] | 0;
                p.updated_at = obj["updated_at"] | 0;
                profiles.push_back(p);
            }
        }
    }
    
    // If no profiles stored, return defaults
    if (profiles.empty()) {
        profiles = getDefaultProfiles();
    }
    
    return profiles;
}

FilamentProfile ConfigManager::getProfile(const String& profile_id) const {
    FilamentProfile empty;
    FilamentProfile builtin_match;
    auto profiles = getProfiles();
    for (const auto& p : profiles) {
        if (p.id != profile_id) continue;
        if (!p.is_builtin) {
            return p;  // custom override wins
        }
        builtin_match = p;
    }
    return builtin_match.id.isEmpty() ? empty : builtin_match;
}

bool ConfigManager::addProfile(const FilamentProfile& profile) {
    auto profiles = getProfiles();

    if (!validateProfileParams(profile.target_temp_c, profile.default_duration_min,
                               profile.target_humidity_pct)) {
        return false;
    }

    bool has_builtin = false;
    for (const auto& p : profiles) {
        if (p.id == profile.id) {
            if (p.is_builtin) {
                has_builtin = true;
            } else {
                return false;  // custom already exists — use update
            }
        }
    }

    // Shadowing a builtin is allowed (custom override with same ID)
    if (!has_builtin) {
        for (const auto& p : profiles) {
            if (p.id == profile.id) return false;
        }
    }

    size_t custom_count = 0;
    for (const auto& p : profiles) {
        if (!p.is_builtin) ++custom_count;
    }
    if (custom_count >= kMaxCustomProfiles) {
        return false;
    }

    FilamentProfile to_save = profile;
    to_save.is_builtin = false;
    to_save.created_at = millis();
    to_save.updated_at = to_save.created_at;
    profiles.push_back(to_save);
    return saveProfiles(profiles);
}

bool ConfigManager::updateProfile(const FilamentProfile& profile) {
    auto profiles = getProfiles();

    if (!validateProfileParams(profile.target_temp_c, profile.default_duration_min,
                               profile.target_humidity_pct)) {
        return false;
    }

    // Update existing custom entry
    for (auto& p : profiles) {
        if (p.id == profile.id && !p.is_builtin) {
            p.name_pt = profile.name_pt;
            p.name_en = profile.name_en;
            p.target_temp_c = profile.target_temp_c;
            p.default_duration_min = profile.default_duration_min;
            p.target_humidity_pct = profile.target_humidity_pct;
            p.updated_at = millis();
            return saveProfiles(profiles);
        }
    }

    // Builtin ID with no custom yet → create override
    for (const auto& p : profiles) {
        if (p.id == profile.id && p.is_builtin) {
            FilamentProfile override_profile = profile;
            override_profile.is_builtin = false;
            override_profile.created_at = millis();
            override_profile.updated_at = override_profile.created_at;
            size_t custom_count = 0;
            for (const auto& existing : profiles) {
                if (!existing.is_builtin) ++custom_count;
            }
            if (custom_count >= kMaxCustomProfiles) return false;
            profiles.push_back(override_profile);
            return saveProfiles(profiles);
        }
    }
    return false;
}

bool ConfigManager::deleteProfile(const String& profile_id) {
    auto profiles = getProfiles();
    
    for (auto it = profiles.begin(); it != profiles.end(); ++it) {
        if (it->id == profile_id) {
            if (it->is_builtin) return false; // Can't delete builtin
            profiles.erase(it);
            return saveProfiles(profiles);
        }
    }
    return false;
}

void ConfigManager::resetProfilesToDefaults() {
    auto profiles = getDefaultProfiles();
    saveProfiles(profiles);
}

bool ConfigManager::saveProfiles(const std::vector<FilamentProfile>& profiles) {
    if (!initialized_) return false;
    
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    
    for (const auto& p : profiles) {
        JsonObject obj = arr.add<JsonObject>();
        obj["id"] = p.id;
        obj["name_pt"] = p.name_pt;
        obj["name_en"] = p.name_en;
        obj["target_temp_c"] = p.target_temp_c;
        obj["default_duration_min"] = p.default_duration_min;
        obj["target_humidity_pct"] = p.target_humidity_pct;
        obj["is_builtin"] = p.is_builtin;
        obj["created_at"] = p.created_at;
        obj["updated_at"] = p.updated_at;
    }
    
    String json;
    serializeJson(doc, json);
    return writeString(KEY_PROFILES, json);
}

std::vector<FilamentProfile> ConfigManager::getDefaultProfiles() {
    std::vector<FilamentProfile> profiles;
    
    FilamentProfile pla;
    pla.id = "pla";
    pla.name_pt = "PLA";
    pla.name_en = "PLA";
    pla.target_temp_c = 50.0f;
    pla.default_duration_min = 240;
    pla.target_humidity_pct = 15.0f;
    pla.is_builtin = true;
    pla.created_at = 0;
    pla.updated_at = 0;

    FilamentProfile petg;
    petg.id = "petg";
    petg.name_pt = "PETG";
    petg.name_en = "PETG";
    petg.target_temp_c = 65.0f;
    petg.default_duration_min = 240;
    petg.target_humidity_pct = 15.0f;
    petg.is_builtin = true;
    petg.created_at = 0;
    petg.updated_at = 0;

    FilamentProfile abs;
    abs.id = "abs";
    abs.name_pt = "ABS";
    abs.name_en = "ABS";
    abs.target_temp_c = 80.0f;
    abs.default_duration_min = 120;
    abs.target_humidity_pct = 10.0f;
    abs.is_builtin = true;
    abs.created_at = 0;
    abs.updated_at = 0;

    FilamentProfile tpu;
    tpu.id = "tpu";
    tpu.name_pt = "TPU";
    tpu.name_en = "TPU";
    tpu.target_temp_c = 45.0f;
    tpu.default_duration_min = 240;
    tpu.target_humidity_pct = 20.0f;
    tpu.is_builtin = true;
    tpu.created_at = 0;
    tpu.updated_at = 0;

    FilamentProfile nylon;
    nylon.id = "nylon";
    nylon.name_pt = "Nylon";
    nylon.name_en = "Nylon";
    nylon.target_temp_c = 70.0f;
    nylon.default_duration_min = 360;
    nylon.target_humidity_pct = 10.0f;
    nylon.is_builtin = true;
    nylon.created_at = 0;
    nylon.updated_at = 0;
    
    profiles.push_back(pla);
    profiles.push_back(petg);
    profiles.push_back(abs);
    profiles.push_back(tpu);
    profiles.push_back(nylon);
    
    return profiles;
}

// Generic Object Configuration (Klipper-style)
bool ConfigManager::setObjectConfig(const String& object_name, const JsonObject& config) {
    if (!initialized_) return false;
    
    // Read existing objects
    std::vector<String> objects = listObjectConfigs();
    bool exists = false;
    
    // Remove if exists (replace strategy)
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if (*it == object_name) {
            exists = true;
            objects.erase(it);
            break;
        }
    }
    
    // Add new object to list
    objects.push_back(object_name);
    writeJsonArray(KEY_OBJECTS, objects);
    
    // Save object config
    String key = "obj_" + object_name;
    String json;
    serializeJson(config, json);
    return writeString(key.c_str(), json);
}

JsonObject ConfigManager::getObjectConfig(const String& object_name) {
    JsonDocument doc;
    if (!initialized_) return doc.as<JsonObject>();
    
    String key = "obj_" + object_name;
    String json;
    if (prefs_.getString(key.c_str(), json)) {
        deserializeJson(doc, json);
    }
    return doc.as<JsonObject>();
}

std::vector<String> ConfigManager::listObjectConfigs() const {
    std::vector<String> objects;
    if (!initialized_) return objects;
    readJsonArray(KEY_OBJECTS, objects);
    return objects;
}

void ConfigManager::factoryReset() {
    if (!initialized_) return;
    prefs_.clear();
}

String ConfigManager::toJson() const {
    JsonDocument doc;
    
    // WiFi
    JsonObject wifi_obj = doc.createNestedObject("wifi");
    WifiConfig wifi = getWifiConfig();
    wifi_obj["ssid"] = wifi.ssid;
    wifi_obj["password"] = wifi.password;
    wifi_obj["valid"] = wifi.valid;
    wifi_obj["use_static_ip"] = wifi.use_static_ip;
    if (!wifi.ip.isEmpty()) {
        wifi_obj["ip"] = wifi.ip;
        wifi_obj["gateway"] = wifi.gateway;
        wifi_obj["netmask"] = wifi.netmask;
        wifi_obj["dns"] = wifi.dns;
        JsonObject sip = wifi_obj.createNestedObject("static_ip");
        sip["ip"] = wifi.ip;
        sip["gateway"] = wifi.gateway;
        sip["netmask"] = wifi.netmask;
        sip["dns"] = wifi.dns;
    }
    
    // Sensor
    JsonObject sensor_obj = doc.createNestedObject("sensor");
    SensorConfig sensor = getSensorConfig();
    sensor_obj["type"] = sensor.type;
    sensor_obj["is_integrated"] = sensor.is_integrated;
    sensor_obj["i2c_bus"] = sensor.i2c_bus;
    sensor_obj["i2c_address"] = sensor.i2c_address;
    sensor_obj["gpio_pin"] = sensor.gpio_pin;
    sensor_obj["sda_pin"] = sensor.sda_pin;
    sensor_obj["scl_pin"] = sensor.scl_pin;
    
    // Actuator
    JsonObject actuator_obj = doc.createNestedObject("actuator");
    ActuatorConfig actuator = getActuatorConfig();
    actuator_obj["heater_type"] = actuator.heater_type;
    actuator_obj["heater_pin"] = actuator.heater_pin;
    actuator_obj["heater_pwm_freq"] = actuator.heater_pwm_freq;
    actuator_obj["heater_max_power_pct"] = actuator.heater_max_power_pct;
    actuator_obj["heater_max_temp_c"] = actuator.heater_max_temp_c;
    actuator_obj["fan_type"] = actuator.fan_type;
    actuator_obj["fan_mode"] = actuator.fan_mode;
    actuator_obj["fan_pin"] = actuator.fan_pin;
    actuator_obj["fan_pwm_freq"] = actuator.fan_pwm_freq;
    actuator_obj["fan_duty_pct"] = actuator.fan_duty_pct;
    actuator_obj["cooldown_duration_sec"] = actuator.cooldown_duration_sec;
    JsonArray curve_arr = actuator_obj.createNestedArray("fan_speed_curve");
    for (const auto& pt : actuator.fan_speed_curve) {
        JsonObject obj = curve_arr.add<JsonObject>();
        obj["temp_c"] = pt.temp_c;
        obj["power_pct"] = pt.power_pct;
    }
    
    // Display
    JsonObject display_obj = doc.createNestedObject("display");
    DisplayConfig display = getDisplayConfig();
    display_obj["enabled"] = display.enabled;
    display_obj["driver"] = display.driver;
    display_obj["bus_type"] = display.bus_type;
    display_obj["width"] = display.width;
    display_obj["height"] = display.height;
    display_obj["rotation"] = display.rotation;
    display_obj["spi_mosi"] = display.spi_mosi;
    display_obj["spi_sclk"] = display.spi_sclk;
    display_obj["spi_cs"] = display.spi_cs;
    display_obj["dc_pin"] = display.dc_pin;
    display_obj["rst_pin"] = display.rst_pin;
    display_obj["backlight_pin"] = display.backlight_pin;
    display_obj["i2c_sda"] = display.i2c_sda;
    display_obj["i2c_scl"] = display.i2c_scl;
    display_obj["i2c_address"] = display.i2c_address;
    display_obj["font_scaling"] = display.font_scaling;
    display_obj["compact_mode"] = display.compact_mode;
    
    JsonArray fields_arr = display_obj.createNestedArray("fields");
    for (const String& f : display.fields) {
        fields_arr.add(f);
    }
    
    // PID
    JsonObject pid_obj = doc.createNestedObject("pid");
    PidConfig pid = getPidConfig();
    pid_obj["kp"] = pid.kp;
    pid_obj["ki"] = pid.ki;
    pid_obj["kd"] = pid.kd;
    pid_obj["calibrated"] = pid.calibrated;
    
    JsonArray profiles_arr = doc.createNestedArray("profiles");
    for (const auto& p : getProfiles()) {
        JsonObject obj = profiles_arr.add<JsonObject>();
        obj["id"] = p.id;
        obj["name_pt"] = p.name_pt;
        obj["name_en"] = p.name_en;
        obj["target_temp_c"] = p.target_temp_c;
        obj["default_duration_min"] = p.default_duration_min;
        obj["target_humidity_pct"] = p.target_humidity_pct;
        obj["is_builtin"] = p.is_builtin;
        obj["created_at"] = p.created_at;
        obj["updated_at"] = p.updated_at;
    }
    
    JsonArray objects_arr = doc.createNestedArray("objects");
    for (const String& obj : listObjectConfigs()) {
        objects_arr.add(obj);
    }
    
    String json;
    serializeJson(doc, json);
    return json;
}

bool ConfigManager::fromJson(const String& json) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        return false;
    }
    
    // WiFi
    if (doc["wifi"].is<JsonObject>()) {
        JsonObject wifiObj = doc["wifi"].as<JsonObject>();
        WifiConfig wifi;
        wifi.ssid = wifiObj["ssid"] | "";
        wifi.password = wifiObj["password"] | "";
        wifi.valid = !wifi.ssid.isEmpty();
        if (wifiObj["static_ip"].is<JsonObject>()) {
            JsonObject sip = wifiObj["static_ip"].as<JsonObject>();
            wifi.ip = sip["ip"] | "";
            wifi.gateway = sip["gateway"] | "";
            wifi.netmask = sip["netmask"] | "255.255.255.0";
            wifi.dns = sip["dns"] | "";
        } else {
            wifi.ip = wifiObj["ip"] | "";
            wifi.gateway = wifiObj["gateway"] | "";
            wifi.netmask = wifiObj["netmask"] | "255.255.255.0";
            wifi.dns = wifiObj["dns"] | "";
        }
        wifi.use_static_ip = wifiObj["use_static_ip"] | !wifi.ip.isEmpty();
        setWifiConfig(wifi);
    }
    
    // Sensor
    if (doc["sensor"].is<JsonObject>()) {
        JsonObject sensorObj = doc["sensor"].as<JsonObject>();
        SensorConfig sensor;
        sensor.type = sensorObj["type"] | "sht31";
        sensor.is_integrated = sensorObj["is_integrated"] | true;
        sensor.i2c_bus = sensorObj["i2c_bus"] | 0;
        sensor.i2c_address = sensorObj["i2c_address"] | 0x44;
        sensor.gpio_pin = sensorObj["gpio_pin"] | -1;
        sensor.sda_pin = sensorObj["sda_pin"] | 21;
        sensor.scl_pin = sensorObj["scl_pin"] | 22;
        setSensorConfig(sensor);
    }
    
    // Actuator
    if (doc["actuator"].is<JsonObject>()) {
        JsonObject actuatorObj = doc["actuator"].as<JsonObject>();
        ActuatorConfig actuator;
        actuator.heater_type = actuatorObj["heater_type"] | "mosfet_pwm";
        actuator.heater_pin = actuatorObj["heater_pin"] | 25;
        actuator.heater_pwm_freq = actuatorObj["heater_pwm_freq"] | 1000;
        actuator.heater_max_power_pct = actuatorObj["heater_max_power_pct"] | 100;
        actuator.heater_max_temp_c = actuatorObj["heater_max_temp_c"] | 0.0f;
        actuator.fan_type = actuatorObj["fan_type"] | "fan_pwm";
        actuator.fan_mode = actuatorObj["fan_mode"] | "independent_pwm";
        actuator.fan_pin = actuatorObj["fan_pin"] | 26;
        actuator.fan_pwm_freq = actuatorObj["fan_pwm_freq"] | 5000;
        actuator.fan_duty_pct = actuatorObj["fan_duty_pct"] | 80.0f;
        actuator.cooldown_duration_sec = actuatorObj["cooldown_duration_sec"] | 30;
        actuator.fan_speed_curve.clear();
        if (actuatorObj["fan_speed_curve"].is<JsonArray>()) {
            for (JsonVariant v : actuatorObj["fan_speed_curve"].as<JsonArray>()) {
                FanCurvePoint pt;
                if (v.is<JsonObject>()) {
                    pt.temp_c = v["temp_c"] | 0.0f;
                    pt.power_pct = v["power_pct"] | 80.0f;
                } else {
                    pt.power_pct = v.as<float>();
                }
                actuator.fan_speed_curve.push_back(pt);
            }
        }
        setActuatorConfig(actuator);
    }
    
    // Display
    if (doc["display"].is<JsonObject>()) {
        JsonObject displayObj = doc["display"].as<JsonObject>();
        DisplayConfig display;
        display.enabled = displayObj["enabled"] | false;
        display.driver = displayObj["driver"] | "auto";
        display.bus_type = displayObj["bus_type"] | "i2c";
        display.width = displayObj["width"] | 128;
        display.height = displayObj["height"] | 64;
        display.rotation = displayObj["rotation"] | 0;
        display.spi_mosi = displayObj["spi_mosi"] | -1;
        display.spi_sclk = displayObj["spi_sclk"] | -1;
        display.spi_cs = displayObj["spi_cs"] | -1;
        display.dc_pin = displayObj["dc_pin"] | -1;
        display.rst_pin = displayObj["rst_pin"] | -1;
        display.backlight_pin = displayObj["backlight_pin"] | -1;
        display.i2c_sda = displayObj["i2c_sda"] | 21;
        display.i2c_scl = displayObj["i2c_scl"] | 22;
        display.i2c_address = displayObj["i2c_address"] | 0x3C;
        display.refresh_rate_hz = displayObj["refresh_rate_hz"] | 1;
        display.font_scaling = displayObj["font_scaling"] | "auto";
        display.compact_mode = displayObj["compact_mode"] | false;
        
        JsonArray fields = displayObj["fields"].as<JsonArray>();
        display.fields.clear();
        for (JsonVariant v : fields) {
            display.fields.push_back(v.as<String>());
        }
        setDisplayConfig(display);
    }
    
    // PID
    if (doc["pid"].is<JsonObject>()) {
        JsonObject pidObj = doc["pid"].as<JsonObject>();
        PidConfig pid;
        pid.kp = pidObj["kp"] | 0.0f;
        pid.ki = pidObj["ki"] | 0.0f;
        pid.kd = pidObj["kd"] | 0.0f;
        pid.calibrated = pidObj["calibrated"] | false;
        setPidConfig(pid);
    }
    
    // Profiles
    if (doc["profiles"].is<JsonArray>()) {
        std::vector<FilamentProfile> profiles;
        JsonArray arr = doc["profiles"].as<JsonArray>();
        for (JsonVariant v : arr) {
            JsonObject obj = v.as<JsonObject>();
            FilamentProfile p;
            p.id = obj["id"] | "";
            p.name_pt = obj["name_pt"] | "";
            p.name_en = obj["name_en"] | "";
            p.target_temp_c = obj["target_temp_c"] | 50.0f;
            p.default_duration_min = obj["default_duration_min"] | 240;
            p.target_humidity_pct = obj["target_humidity_pct"] | 15.0f;
            p.is_builtin = obj["is_builtin"] | true;
            p.created_at = obj["created_at"] | 0;
            p.updated_at = obj["updated_at"] | 0;
            profiles.push_back(p);
        }
        saveProfiles(profiles);
    }
    
    return true;
}

// Template helpers
template<typename T>
bool ConfigManager::readStruct(const char* key, T& value) {
    if (!initialized_) return false;
    size_t size = prefs_.getBytesLength(key);
    if (size != sizeof(T)) return false;
    return prefs_.getBytes(key, &value, sizeof(T)) == sizeof(T);
}

template<typename T>
bool ConfigManager::writeStruct(const char* key, const T& value) {
    if (!initialized_) return false;
    return prefs_.putBytes(key, &value, sizeof(T)) == sizeof(T);
}

bool ConfigManager::readString(const char* key, String& value) {
    if (!initialized_) return false;
    value = prefs_.getString(key, "");
    return !value.isEmpty();
}

bool ConfigManager::writeString(const char* key, const String& value) {
    if (!initialized_) return false;
    return prefs_.putString(key, value);
}

bool ConfigManager::readJsonArray(const char* key, std::vector<String>& array) const {
    if (!initialized_) return false;
    String json = prefs_.getString(key, "[]");
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return false;
    array.clear();
    JsonArray arr = doc.as<JsonArray>();
    for (JsonVariant v : arr) {
        array.push_back(v.as<String>());
    }
    return true;
}

bool ConfigManager::writeJsonArray(const char* key, const std::vector<String>& array) {
    if (!initialized_) return false;
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const String& s : array) {
        arr.add(s);
    }
    String json;
    serializeJson(doc, json);
    return prefs_.putString(key, json);
}

// Explicit template instantiations
template bool ConfigManager::readStruct<WifiConfig>(const char*, WifiConfig&);
template bool ConfigManager::writeStruct<WifiConfig>(const char*, const WifiConfig&);

} // namespace filament_dryer