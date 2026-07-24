/**
 * WifiManager - Implementation
 */
#include "WifiManager.hpp"

namespace filament_dryer {

WifiManager::WifiManager() {}

WifiManager::~WifiManager() {
    if (ap_active_) {
        WiFi.softAPdisconnect(true);
    }
}

bool WifiManager::begin() {
    Serial.println("[WifiManager] Starting...");
    
    if (!prefs_.begin("filament_dryer", true)) {
        Serial.println("[WifiManager] ERROR: Failed to open NVS");
        return false;
    }
    
    loadConfig();
    
    if (current_config_.valid && !current_config_.ssid.isEmpty()) {
        updateStatus(Status::CONNECTING);
        if (!connect()) {
            Serial.println("[WifiManager] STA connection failed, starting AP");
            startAP();
        }
    } else {
        Serial.println("[WifiManager] No WiFi config, starting AP");
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
    Serial.printf("[WifiManager] Connecting to '%s'...\n", ssid.c_str());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_connected_ = true;
        updateStatus(Status::CONNECTED);
        Serial.printf("[WifiManager] Connected! IP: %s, RSSI: %d\n", 
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    }
    
    WiFi.disconnect(true);
    wifi_connected_ = false;
    return false;
}

void WifiManager::startAP() {
    Serial.println("[WifiManager] Starting AP 'philarmony'...");
    
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    bool result = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL);
    
    if (result) {
        ap_active_ = true;
        updateStatus(Status::AP_ACTIVE);
        Serial.printf("[WifiManager] AP started at %s\n", AP_IP.toString().c_str());
    } else {
        Serial.println("[WifiManager] ERROR: Failed to start AP");
    }
}

void WifiManager::stopAP() {
    if (ap_active_) {
        WiFi.softAPdisconnect(true);
        ap_active_ = false;
        Serial.println("[WifiManager] AP stopped");
    }
}

void WifiManager::loop() {
    // Handle reconnection for STA mode
    if (status_ == Status::CONNECTED) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WifiManager] WiFi disconnected, attempting reconnect...");
            wifi_connected_ = false;
            
            if (retry_count_ < 5) {
                retry_count_++;
                retry_delay_ms_ = min(retry_delay_ms_ * 2, 60000); // Exponential backoff
                last_retry_ = millis();
                updateStatus(Status::CONNECTING);
            } else {
                // Max retries reached, fall back to AP
                Serial.println("[WifiManager] Max retries reached, falling back to AP");
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
            if (!connect()) {
                Serial.println("[WifiManager] Reconnect failed");
            }
        }
    }
}

WifiManager::WifiConfig WifiManager::getConfig() const {
    return current_config_;
}

bool WifiManager::setConfig(const WifiConfig& config) {
    current_config_ = config;
    current_config_.valid = !config.ssid.isEmpty();
    
    if (saveConfig()) {
        // Try to connect with new config
        if (connect()) {
            stopAP();
        } else {
            startAP();
        }
        return true;
    }
    return false;
}

void WifiManager::clearConfig() {
    current_config_ = { "", "", false };
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
    StaticJsonDocument<512> doc;
    deserializeJson(doc, json);
    
    current_config_.ssid = doc["ssid"] | "";
    current_config_.password = doc["password"] | "";
    current_config_.valid = doc["valid"] | false;
    
    Serial.printf("[WifiManager] Loaded config: ssid='%s', valid=%d\n", 
                  current_config_.ssid.c_str(), current_config_.valid);
}

bool WifiManager::saveConfig() {
    StaticJsonDocument<512> doc;
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