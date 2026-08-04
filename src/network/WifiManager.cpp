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

uint32_t WifiManager::connectTimeoutMs() const {
    return ever_connected_ ? RECONNECT_TIMEOUT_MS : CONNECT_TIMEOUT_MS;
}

void WifiManager::beginConnectAsync(const String& ssid, const String& password) {
    wifi_connected_ = false;
    connect_start_ms_ = millis();
    last_retry_ = connect_start_ms_;
    updateStatus(Status::CONNECTING);

    WiFi.mode(ap_active_ ? WIFI_AP_STA : WIFI_STA);
    WiFi.disconnect(false);

    if (current_config_.use_static_ip && !current_config_.ip.isEmpty()) {
        IPAddress ip, gateway, netmask, dns;
        if (ip.fromString(current_config_.ip) &&
            gateway.fromString(current_config_.gateway.isEmpty()
                                   ? current_config_.ip
                                   : current_config_.gateway) &&
            netmask.fromString(current_config_.netmask.isEmpty()
                                   ? "255.255.255.0"
                                   : current_config_.netmask)) {
            if (!current_config_.dns.isEmpty() && dns.fromString(current_config_.dns)) {
                WiFi.config(ip, gateway, netmask, dns);
            } else {
                WiFi.config(ip, gateway, netmask);
            }
        }
    }

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
    }
    ap_active_ = false;
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
        const wl_status_t st = WiFi.status();
        if (st == WL_CONNECTED) {
            wifi_connected_ = true;
            ever_connected_ = true;
            retry_count_ = 0;
            retry_delay_ms_ = 5000;
            stopAP();
            updateStatus(Status::CONNECTED);
            return;
        }

        // Fail fast on hard auth errors so AP meets SC-01 (<5s)
        if (st == WL_NO_SSID_AVAIL || st == WL_CONNECT_FAILED || st == WL_CONNECTION_LOST) {
            WiFi.disconnect(true);
            wifi_connected_ = false;
            if (!ap_active_) {
                startAP();
            } else {
                updateStatus(Status::AP_ACTIVE);
            }
            return;
        }

        if (millis() - connect_start_ms_ >= connectTimeoutMs()) {
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
                beginConnectAsync(current_config_.ssid, current_config_.password);
            } else {
                startAP();
            }
        } else {
            retry_count_ = 0;
            retry_delay_ms_ = 5000;
        }
        return;
    }

    if (status_ == Status::AP_ACTIVE || status_ == Status::DISCONNECTED) {
        if (millis() - last_retry_ >= retry_delay_ms_ && current_config_.valid &&
            !current_config_.ssid.isEmpty() && !wifi_connected_) {
            // Background STA retry while AP stays up
            beginConnectAsync(current_config_.ssid, current_config_.password);
            last_retry_ = millis();
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
    ever_connected_ = false; // new creds: use fail-fast timeout
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
    if (config_mgr_) {
        current_config_ = config_mgr_->getWifiConfig();
        return;
    }
    // Fallback only if ConfigManager was not wired (should not happen in production)
    current_config_ = WifiConfig{};
}

bool WifiManager::saveConfig() {
    if (config_mgr_) {
        return config_mgr_->setWifiConfig(current_config_);
    }
    return false;
}

void WifiManager::updateStatus(Status new_status) {
    if (status_ != new_status) {
        status_ = new_status;
        if (status_cb_) status_cb_(new_status);
    }
}

} // namespace filament_dryer
