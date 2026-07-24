/**
 * WifiManager - WiFi STA connection with AP fallback
 */
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

namespace filament_dryer {

struct WifiConfig {
    String ssid;
    String password;
    bool valid = false;
};

class WifiManager {
public:
    static constexpr const char* AP_SSID = "philarmony";
    static constexpr const char* AP_PASSWORD = "philarmony";
    static constexpr uint8_t AP_CHANNEL = 1;
    static constexpr IPAddress AP_IP(192, 168, 4, 1);
    static constexpr IPAddress AP_GATEWAY(192, 168, 4, 1);
    static constexpr IPAddress AP_SUBNET(255, 255, 255, 0);
    
    WifiManager();
    ~WifiManager();
    
    bool begin();
    bool connect();
    void startAP();
    void stopAP();
    void loop();
    
    bool isConnected() const { return wifi_connected_; }
    bool isAPActive() const { return ap_active_; }
    String getLocalIP() const { return WiFi.localIP().toString(); }
    String getAPIP() const { return WiFi.softAPIP().toString(); }
    int32_t getRSSI() const { return WiFi.RSSI(); }
    
    WifiConfig getConfig() const { return current_config_; }
    bool setConfig(const WifiConfig& config);
    void clearConfig();
    
    // Connection status
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
    uint8_t retry_count_ = 0;
    uint16_t retry_delay_ms_ = 5000;
    StatusCallback status_cb_ = nullptr;
    
    void loadConfig();
    bool saveConfig();
    void updateStatus(Status new_status);
    bool connectToWiFi(const String& ssid, const String& password);
};

} // namespace filament_dryer