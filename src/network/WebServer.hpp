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

class WebServer {
public:
    WebServer(uint16_t port = 80);
    ~WebServer();
    
    bool begin(ConfigManager* config_mgr, LogManager* log_mgr);
    
    // Template for captive portal
    static const char* getCaptivePortalHTML();
    static const char* getConfigPageHTML();
    
private:
    uint16_t port_;
    AsyncWebServer* server_ = nullptr;
    ConfigManager* config_mgr_ = nullptr;
    LogManager* log_mgr_ = nullptr;
    
    // Route handlers
    void setupRoutes();
    void handleCaptivePortal(AsyncWebServerRequest* request);
    void handleRoot(AsyncWebServerRequest* request);
    void handleInfo(AsyncWebServerRequest* request);
    void handleWifiConfigPost(AsyncWebServerRequest* request);
    void handleLogDownload(AsyncWebServerRequest* request, bool drying_log);
    void handleNotFound(AsyncWebServerRequest* request);
};

} // namespace filament_dryer