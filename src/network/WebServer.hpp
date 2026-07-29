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
 * WebServer - Async HTTP Server for configuration and log downloads
 */
#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

namespace filament_dryer {

class ConfigManager;
class LogManager;
class HardwareConfigParser;
class DriverRegistry;
class WifiManager;
class StateMachine;
class PluginManager;

class WebServer {
public:
    using HardwareReloadCallback = void(*)();

    WebServer(uint16_t port = 80);
    ~WebServer();
    
    bool begin(ConfigManager* config_mgr, LogManager* log_mgr,
               HardwareConfigParser* hw_parser = nullptr,
               DriverRegistry* driver_registry = nullptr,
               WifiManager* wifi_mgr = nullptr,
               StateMachine* state_machine = nullptr);

    void attachWebSocket(AsyncWebSocket* ws);
    void setHardwareReloadCallback(HardwareReloadCallback cb) { hardware_reload_cb_ = cb; }
    void setStateMachine(StateMachine* sm) { state_machine_ = sm; }
    void setPluginManager(PluginManager* pm) { plugin_mgr_ = pm; }
    
    // Template for captive portal
    static const char* getCaptivePortalHTML();
    static const char* getConfigPageHTML();
    
private:
    uint16_t port_;
    AsyncWebServer* server_ = nullptr;
    ConfigManager* config_mgr_ = nullptr;
    LogManager* log_mgr_ = nullptr;
    HardwareConfigParser* hw_parser_ = nullptr;
    DriverRegistry* driver_registry_ = nullptr;
    WifiManager* wifi_mgr_ = nullptr;
    StateMachine* state_machine_ = nullptr;
    PluginManager* plugin_mgr_ = nullptr;
    HardwareReloadCallback hardware_reload_cb_ = nullptr;
    
    // Route handlers
    void setupRoutes();
    void handleCaptivePortal(AsyncWebServerRequest* request);
    void handleRoot(AsyncWebServerRequest* request);
    void handleInfo(AsyncWebServerRequest* request);
    void handleWifiConfigPost(AsyncWebServerRequest* request);
    void handleWifiConfigApply(AsyncWebServerRequest* request, const String& ssid, const String& password);
    void handleLogDownload(AsyncWebServerRequest* request, bool drying_log);
    void handleHardwareConfigGet(AsyncWebServerRequest* request);
    void handleHardwareConfigPostBody(AsyncWebServerRequest* request, const String& body);
    void handleNotFound(AsyncWebServerRequest* request);
    void buildKlipperHardwareConfig(JsonObject& root);
    
    // Helpers
    void sendJson(AsyncWebServerRequest* request, int code, const String& json);
    void sendError(AsyncWebServerRequest* request, int code, const String& error);
};

} // namespace filament_dryer