/**
 * WifiManager - WiFi STA connection with AP fallback (non-blocking)
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "../core/ConfigManager.hpp"

namespace filament_dryer {

class WifiManager {
public:
    static constexpr const char* AP_SSID = "philarmony";
    static constexpr const char* AP_PASSWORD = "philarmony";
    static constexpr uint8_t AP_CHANNEL = 1;
    static constexpr uint32_t CONNECT_TIMEOUT_MS = 20000;
    static const IPAddress AP_IP;
    static const IPAddress AP_GATEWAY;
    static const IPAddress AP_SUBNET;
    
    WifiManager();
    ~WifiManager();
    
    bool begin();
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

private:
    Preferences prefs_;
    WifiConfig current_config_;
    Status status_ = Status::DISCONNECTED;
    bool wifi_connected_ = false;
    bool ap_active_ = false;
    uint32_t last_retry_ = 0;
    uint32_t connect_start_ms_ = 0;
    uint8_t retry_count_ = 0;
    uint16_t retry_delay_ms_ = 5000;
    StatusCallback status_cb_ = nullptr;
    DNSServer dns_server_;
    bool dns_active_ = false;
    
    void loadConfig();
    bool saveConfig();
    void updateStatus(Status new_status);
    void beginConnectAsync(const String& ssid, const String& password);
    void startDnsSinkhole();
    void stopDnsSinkhole();
};

} // namespace filament_dryer
