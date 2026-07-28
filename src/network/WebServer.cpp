#include "WebServer.hpp"

#include "../core/ConfigManager.hpp"
#include "../core/LogManager.hpp"

namespace filament_dryer {

WebServer::WebServer(uint16_t port) : port_(port) {}

WebServer::~WebServer() {
    if (server_) {
        server_->end();
        delete server_;
    }
}

bool WebServer::begin(ConfigManager* config_mgr, LogManager* log_mgr,
                      HardwareConfigParser* hw_parser,
                      DriverRegistry* driver_registry) {
    config_mgr_ = config_mgr;
    log_mgr_ = log_mgr;
    hw_parser_ = hw_parser;
    driver_registry_ = driver_registry;

    if (!server_) {
        server_ = new AsyncWebServer(port_);
        setupRoutes();
        server_->begin();
    }

    return true;
}

void WebServer::setupRoutes() {
    if (!server_) {
        return;
    }

    server_->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) { handleRoot(request); });
    server_->on("/info", HTTP_GET, [this](AsyncWebServerRequest* request) { handleInfo(request); });
    server_->on("/api/wifi/config", HTTP_POST, [this](AsyncWebServerRequest* request) { handleWifiConfigPost(request); });
    server_->on("/log/drying", HTTP_GET, [this](AsyncWebServerRequest* request) { handleLogDownload(request, true); });
    server_->on("/log/system", HTTP_GET, [this](AsyncWebServerRequest* request) { handleLogDownload(request, false); });
    server_->onNotFound([this](AsyncWebServerRequest* request) { handleNotFound(request); });
}

void WebServer::handleCaptivePortal(AsyncWebServerRequest* request) {
    handleRoot(request);
}

void WebServer::handleRoot(AsyncWebServerRequest* request) {
    request->send(200, "text/html", getConfigPageHTML());
}

void WebServer::handleInfo(AsyncWebServerRequest* request) {
    String json = "{\"status\":\"ok\"}";
    sendJson(request, 200, json);
}

void WebServer::handleWifiConfigPost(AsyncWebServerRequest* request) {
    if (!config_mgr_) {
        sendError(request, 500, "Config manager unavailable");
        return;
    }

    WifiConfig config = config_mgr_->getWifiConfig();
    if (request->hasParam("ssid", true)) {
        config.ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
        config.password = request->getParam("password", true)->value();
    }
    config.valid = !config.ssid.isEmpty();

    if (!config_mgr_->setWifiConfig(config)) {
        sendError(request, 500, "Failed to persist WiFi configuration");
        return;
    }

    sendJson(request, 200, "{\"status\":\"saved\"}");
}

void WebServer::handleLogDownload(AsyncWebServerRequest* request, bool drying_log) {
    if (!log_mgr_) {
        sendError(request, 500, "Log manager unavailable");
        return;
    }

    String body;
    const bool ok = drying_log ? log_mgr_->getDryingLog(body) : log_mgr_->getSystemLog(body);
    if (!ok) {
        sendError(request, 404, "Log not available");
        return;
    }

    request->send(200, "text/plain", body);
}

void WebServer::handleHardwareConfigGet(AsyncWebServerRequest* request) {
    sendJson(request, 200, "{\"status\":\"unsupported\"}");
}

void WebServer::handleHardwareConfigPostBody(AsyncWebServerRequest* request, const String& body) {
    (void)body;
    sendJson(request, 200, "{\"status\":\"unsupported\"}");
}

void WebServer::handleNotFound(AsyncWebServerRequest* request) {
    sendError(request, 404, "Not found");
}

void WebServer::sendJson(AsyncWebServerRequest* request, int code, const String& json) {
    request->send(code, "application/json", json);
}

void WebServer::sendError(AsyncWebServerRequest* request, int code, const String& error) {
    JsonDocument doc;
    doc["error"] = error;
    String json;
    serializeJson(doc, json);
    sendJson(request, code, json);
}

const char* WebServer::getCaptivePortalHTML() {
    return getConfigPageHTML();
}

const char* WebServer::getConfigPageHTML() {
    return "<!doctype html><html><body><h1>Philarmony</h1></body></html>";
}

}  // namespace filament_dryer
