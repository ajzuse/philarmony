/**
 * WifiManager - Implementation
 */
#include "WifiManager.hpp"

namespace filament_dryer {

const IPAddress WifiManager::AP_IP(192, 168, 4, 1);
const IPAddress WifiManager::AP_GATEWAY(192, 168, 4, 1);
const IPAddress WifiManager::AP_SUBNET(255, 255, 255, 0);

WifiManager::WifiManager() {}

WifiManager::~WifiManager() {
    if (ap_active_) {
        WiFi.softAPdisconnect(true);
    }
}

bool WifiManager::begin() {
    if (!prefs_.begin("filament_dryer", true)) {
        return false;
    }
    
    loadConfig();
    
    if (current_config_.valid && !current_config_.ssid.isEmpty()) {
        updateStatus(Status::CONNECTING);
        if (!connect()) {
            startAP();
        }
    } else {
        startAP();
    }
    
    return true;
}

bool WifiManager::connect() {
    if (current_config_.ssid.isEmpty()) {
        return false;
    }
    
    return connectToWiFi(current_config_.ssid, current_config_.password);
}

bool WifiManager::connectToWiFi(const String& ssid, const String& password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_connected_ = true;
        updateStatus(Status::CONNECTED);
        return true;
    }
    
    WiFi.disconnect(true);
    wifi_connected_ = false;
    return false;
}

void WifiManager::startAP() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    bool result = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL);
    
    if (result) {
        ap_active_ = true;
        updateStatus(Status::AP_ACTIVE);
    }
}

void WifiManager::stopAP() {
    if (ap_active_) {
        WiFi.softAPdisconnect(true);
        ap_active_ = false;
    }
}

void WifiManager::loop() {
    // Handle reconnection for STA mode
    if (status_ == Status::CONNECTED) {
        if (WiFi.status() != WL_CONNECTED) {
            wifi_connected_ = false;
            
            if (retry_count_ < 5) {
                retry_count_++;
                retry_delay_ms_ = min(retry_delay_ms_ * 2, 60000); // Exponential backoff
                last_retry_ = millis();
                updateStatus(Status::CONNECTING);
            } else {
                // Max retries reached, fall back to AP
                stopAP();
                startAP();
            }
        } else {
            // Reset retry counter on successful connection
            retry_count_ = 0;
            retry_delay_ms_ = 5000;
        }
    }
    
    // Attempt reconnection if in connecting state
    if (status_ == Status::CONNECTING) {
        if (millis() - last_retry_ >= retry_delay_ms_) {
            connect();
        }
    }
}

bool WifiManager::setConfig(const WifiConfig& config) {
    current_config_ = config;
    current_config_.valid = !config.ssid.isEmpty();
    
    if (!saveConfig()) {
        return false;
    }

    if (connect()) {
        stopAP();
        return true;
    }

    startAP();
    return false;
}

void WifiManager::clearConfig() {
    current_config_ = WifiConfig{};
    saveConfig();
    if (wifi_connected_) {
        WiFi.disconnect(true);
        wifi_connected_ = false;
    }
    if (!ap_active_) {
        startAP();
    }
}

void WifiManager::loadConfig() {
    String json = prefs_.getString("wifi", "{}");
    JsonDocument doc;
    deserializeJson(doc, json);
    
    current_config_.ssid = doc["ssid"] | "";
    current_config_.password = doc["password"] | "";
    current_config_.valid = doc["valid"] | false;
    
}

bool WifiManager::saveConfig() {
    JsonDocument doc;
    doc["ssid"] = current_config_.ssid;
    doc["password"] = current_config_.password;
    doc["valid"] = current_config_.valid;
    
    String json;
    serializeJson(doc, json);
    
    prefs_.end();
    if (!prefs_.begin("filament_dryer", false)) return false;
    bool ok = prefs_.putString("wifi", json);
    prefs_.end();
    prefs_.begin("filament_dryer", true);
    
    return ok;
}

void WifiManager::updateStatus(Status new_status) {
    if (status_ != new_status) {
        status_ = new_status;
        if (status_cb_) status_cb_(new_status);
    }
}

} // namespace filament_dryer