/**
 * WifiManager - Non-blocking STA + AP fallback + DNS captive sinkhole
 */
#include "WifiManager.hpp"

namespace filament_dryer {

const IPAddress WifiManager::AP_IP(192, 168, 4, 1);
const IPAddress WifiManager::AP_GATEWAY(192, 168, 4, 1);
const IPAddress WifiManager::AP_SUBNET(255, 255, 255, 0);

WifiManager::WifiManager() {}

WifiManager::~WifiManager() {
    stopDnsSinkhole();
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
        beginConnectAsync(current_config_.ssid, current_config_.password);
    } else {
        startAP();
    }
    
    return true;
}

bool WifiManager::connect() {
    if (current_config_.ssid.isEmpty()) {
        return false;
    }
    beginConnectAsync(current_config_.ssid, current_config_.password);
    return true;
}

void WifiManager::beginConnectAsync(const String& ssid, const String& password) {
    wifi_connected_ = false;
    connect_start_ms_ = millis();
    last_retry_ = connect_start_ms_;
    updateStatus(Status::CONNECTING);

    WiFi.mode(ap_active_ ? WIFI_AP_STA : WIFI_STA);
    WiFi.disconnect(false);
    WiFi.begin(ssid.c_str(), password.c_str());
}

void WifiManager::startAP() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    bool result = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL);
    
    if (result) {
        ap_active_ = true;
        startDnsSinkhole();
        updateStatus(Status::AP_ACTIVE);
    }
}

void WifiManager::stopAP() {
    stopDnsSinkhole();
    if (ap_active_) {
        WiFi.softAPdisconnect(true);
        ap_active_ = false;
    }
}

void WifiManager::startDnsSinkhole() {
    if (dns_active_) {
        return;
    }
    dns_server_.setErrorReplyCode(DNSReplyCode::NoError);
    dns_active_ = dns_server_.start(53, "*", AP_IP);
}

void WifiManager::stopDnsSinkhole() {
    if (dns_active_) {
        dns_server_.stop();
        dns_active_ = false;
    }
}

void WifiManager::loop() {
    if (dns_active_) {
        dns_server_.processNextRequest();
    }

    if (status_ == Status::CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            wifi_connected_ = true;
            retry_count_ = 0;
            retry_delay_ms_ = 5000;
            stopAP();
            updateStatus(Status::CONNECTED);
            return;
        }

        if (millis() - connect_start_ms_ >= CONNECT_TIMEOUT_MS) {
            WiFi.disconnect(true);
            wifi_connected_ = false;
            if (!ap_active_) {
                startAP();
            } else {
                updateStatus(Status::AP_ACTIVE);
            }
        }
        return;
    }

    if (status_ == Status::CONNECTED) {
        if (WiFi.status() != WL_CONNECTED) {
            wifi_connected_ = false;
            
            if (retry_count_ < 5) {
                retry_count_++;
                retry_delay_ms_ = min(retry_delay_ms_ * 2, 60000);
                last_retry_ = millis();
                updateStatus(Status::CONNECTING);
            } else {
                startAP();
            }
        } else {
            retry_count_ = 0;
            retry_delay_ms_ = 5000;
        }
        return;
    }

    if (status_ == Status::CONNECTING || status_ == Status::DISCONNECTED) {
        if (millis() - last_retry_ >= retry_delay_ms_ && current_config_.valid) {
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

    retry_count_ = 0;
    beginConnectAsync(current_config_.ssid, current_config_.password);
    return true;
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
