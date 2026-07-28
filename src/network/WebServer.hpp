/**
 * WebServer - Async HTTP Server for configuration and log downloads
 */
#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

namespace filament_dryer {

class ConfigManager;
class LogManager;
class HardwareConfigParser;
class DriverRegistry;
class WifiManager;

class WebServer {
public:
    using HardwareReloadCallback = void(*)();

    WebServer(uint16_t port = 80);
    ~WebServer();
    
    bool begin(ConfigManager* config_mgr, LogManager* log_mgr,
               HardwareConfigParser* hw_parser = nullptr,
               DriverRegistry* driver_registry = nullptr,
               WifiManager* wifi_mgr = nullptr);

    void attachWebSocket(AsyncWebSocket* ws);
    void setHardwareReloadCallback(HardwareReloadCallback cb) { hardware_reload_cb_ = cb; }
    
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
    HardwareReloadCallback hardware_reload_cb_ = nullptr;
    
    // Route handlers
    void setupRoutes();
    void handleCaptivePortal(AsyncWebServerRequest* request);
    void handleRoot(AsyncWebServerRequest* request);
    void handleInfo(AsyncWebServerRequest* request);
    void handleWifiConfigPost(AsyncWebServerRequest* request);
    void handleLogDownload(AsyncWebServerRequest* request, bool drying_log);
    void handleHardwareConfigGet(AsyncWebServerRequest* request);
    void handleHardwareConfigPostBody(AsyncWebServerRequest* request, const String& body);
    void handleNotFound(AsyncWebServerRequest* request);
    
    // Helpers
    void sendJson(AsyncWebServerRequest* request, int code, const String& json);
    void sendError(AsyncWebServerRequest* request, int code, const String& error);
};

} // namespace filament_dryer