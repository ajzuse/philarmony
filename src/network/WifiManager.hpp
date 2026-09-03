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
 * WifiManager - WiFi STA connection with AP fallback (non-blocking)
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "../core/ConfigManager.hpp"
#include "timing_contracts.h"

namespace filament_dryer {

class WifiManager {
public:
    static constexpr const char* AP_SSID = "philarmony";
    static constexpr const char* AP_PASSWORD = "philarmony";
    static constexpr uint8_t AP_CHANNEL = 1;
    /** First STA attempt: fail fast so AP meets SC-01 (<5s). */
    static constexpr uint32_t CONNECT_TIMEOUT_MS = kWifiStaConnectTimeoutMs;
    /** Later reconnect attempts after a prior successful join. */
    static constexpr uint32_t RECONNECT_TIMEOUT_MS = kWifiStaReconnectTimeoutMs;
    static const IPAddress AP_IP;
    static const IPAddress AP_GATEWAY;
    static const IPAddress AP_SUBNET;

    WifiManager();
    ~WifiManager();

    bool begin();
    /** Prefer ConfigManager as sole NVS owner for WiFi credentials. */
    void setConfigManager(ConfigManager* config_mgr) { config_mgr_ = config_mgr; }
    /** Start non-blocking STA connect; completion handled in loop(). */
    bool connect();
    void startAP();
    void stopAP();
    void loop();

    bool isConnected() const { return wifi_connected_; }
    bool isAPActive() const { return ap_active_; }
    bool isConnecting() const { return status_ == Status::CONNECTING; }
    String getLocalIP() const { return WiFi.localIP().toString(); }
    String getAPIP() const { return WiFi.softAPIP().toString(); }
    int32_t getRSSI() const { return WiFi.RSSI(); }

    WifiConfig getConfig() const { return current_config_; }
    /** Persist credentials and start async STA connect. Returns true if save ok. */
    bool setConfig(const WifiConfig& config);
    void clearConfig();

    enum class Status {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        AP_ACTIVE
    };
    Status getStatus() const { return status_; }

    using StatusCallback = void(*)(Status);
    void setStatusCallback(StatusCallback cb) { status_cb_ = cb; }

    /** True after SNTP has been started (clock may still be converging). */
    bool isNtpStarted() const { return ntp_started_; }

private:
    void startNtpSync();
    ConfigManager* config_mgr_ = nullptr;
    WifiConfig current_config_;
    Status status_ = Status::DISCONNECTED;
    bool wifi_connected_ = false;
    bool ap_active_ = false;
    bool ever_connected_ = false;
    uint32_t last_retry_ = 0;
    uint32_t connect_start_ms_ = 0;
    uint8_t retry_count_ = 0;
    uint16_t retry_delay_ms_ = 5000;
    StatusCallback status_cb_ = nullptr;
    DNSServer dns_server_;
    bool dns_active_ = false;
    bool ntp_started_ = false;

    void loadConfig();
    bool saveConfig();
    void updateStatus(Status new_status);
    void beginConnectAsync(const String& ssid, const String& password);
    uint32_t connectTimeoutMs() const;
    void startDnsSinkhole();
    void stopDnsSinkhole();
};

} // namespace filament_dryer
