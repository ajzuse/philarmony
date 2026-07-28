#include "WebServer.hpp"

#include "../core/ConfigManager.hpp"
#include "../core/LogManager.hpp"
#include "../core/HardwareConfigParser.hpp"
#include "WifiManager.hpp"

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
                      DriverRegistry* driver_registry,
                      WifiManager* wifi_mgr) {
    config_mgr_ = config_mgr;
    log_mgr_ = log_mgr;
    hw_parser_ = hw_parser;
    driver_registry_ = driver_registry;
    wifi_mgr_ = wifi_mgr;

    if (!server_) {
        server_ = new AsyncWebServer(port_);
        setupRoutes();
        server_->begin();
    }

    return true;
}

void WebServer::attachWebSocket(AsyncWebSocket* ws) {
    if (server_ && ws) {
        server_->addHandler(ws);
    }
}

void WebServer::setupRoutes() {
    if (!server_) {
        return;
    }

    server_->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) { handleRoot(request); });
    server_->on("/info", HTTP_GET, [this](AsyncWebServerRequest* request) { handleInfo(request); });
    server_->on("/api/wifi/config", HTTP_POST, [this](AsyncWebServerRequest* request) { handleWifiConfigPost(request); });
    server_->on("/api/hardware/config", HTTP_GET, [this](AsyncWebServerRequest* request) { handleHardwareConfigGet(request); });
    server_->on(
        "/api/hardware/config", HTTP_POST,
        [](AsyncWebServerRequest* request) {},
        nullptr,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
            String body;
            body.reserve(len);
            for (size_t i = 0; i < len; ++i) {
                body += static_cast<char>(data[i]);
            }
            handleHardwareConfigPostBody(request, body);
        });
    server_->on("/log/drying", HTTP_GET, [this](AsyncWebServerRequest* request) { handleLogDownload(request, true); });
    server_->on("/log/system", HTTP_GET, [this](AsyncWebServerRequest* request) { handleLogDownload(request, false); });
    // Captive portal detection endpoints (Android/iOS/Windows)
    auto captive = [this](AsyncWebServerRequest* request) { handleCaptivePortal(request); };
    server_->on("/generate_204", HTTP_GET, captive);
    server_->on("/gen_204", HTTP_GET, captive);
    server_->on("/hotspot-detect.html", HTTP_GET, captive);
    server_->on("/library/test/success.html", HTTP_GET, captive);
    server_->on("/connecttest.txt", HTTP_GET, captive);
    server_->on("/ncsi.txt", HTTP_GET, captive);
    server_->on("/fwlink", HTTP_GET, captive);
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

    bool connected = false;
    bool connecting = false;
    if (wifi_mgr_) {
        connected = wifi_mgr_->setConfig(config) && wifi_mgr_->isConnected();
        connecting = wifi_mgr_->isConnecting();
    }

    if (connected) {
        sendJson(request, 200, "{\"status\":\"connected\"}");
    } else if (connecting) {
        sendJson(request, 200, "{\"status\":\"connecting\"}");
    } else {
        sendJson(request, 200, "{\"status\":\"saved\",\"ap_active\":true}");
    }
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
    if (!config_mgr_) {
        sendError(request, 500, "Config manager unavailable");
        return;
    }

    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();

    const SensorConfig sensor = config_mgr_->getSensorConfig();
    JsonObject sensor_obj = root["sensor"].to<JsonObject>();
    sensor_obj["type"] = sensor.type;
    sensor_obj["i2c_address"] = sensor.i2c_address;
    sensor_obj["gpio_pin"] = sensor.gpio_pin;
    sensor_obj["temperature_offset"] = sensor.temperature_offset;
    sensor_obj["temperature_scale"] = sensor.temperature_scale;
    sensor_obj["humidity_offset"] = sensor.humidity_offset;
    sensor_obj["humidity_scale"] = sensor.humidity_scale;

    const ActuatorConfig actuator = config_mgr_->getActuatorConfig();
    JsonObject actuator_obj = root["actuator"].to<JsonObject>();
    actuator_obj["heater_pin"] = actuator.heater_pin;
    actuator_obj["fan_pin"] = actuator.fan_pin;
    actuator_obj["fan_mode"] = actuator.fan_mode;

    const DisplayConfig display = config_mgr_->getDisplayConfig();
    JsonObject display_obj = root["display"].to<JsonObject>();
    display_obj["enabled"] = display.enabled;
    display_obj["driver"] = display.driver;
    display_obj["width"] = display.width;
    display_obj["height"] = display.height;

    root["control"] = config_mgr_->getObjectConfig("control");

    String json;
    serializeJson(doc, json);
    sendJson(request, 200, json);
}

void WebServer::handleHardwareConfigPostBody(AsyncWebServerRequest* request, const String& body) {
    if (!config_mgr_ || !hw_parser_) {
        sendError(request, 500, "Hardware parser unavailable");
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok) {
        sendError(request, 400, "Invalid JSON body");
        return;
    }

    SensorConfig sensor_cfg;
    ActuatorConfig actuator_cfg;
    DisplayConfig display_cfg;
    ControlConfig control_cfg;
    auto result = hw_parser_->parse(doc.as<JsonObject>(), sensor_cfg, actuator_cfg, display_cfg, control_cfg);
    if (!result.valid) {
        sendError(request, 400, result.errors.empty() ? "Validation failed" : result.errors[0]);
        return;
    }

    config_mgr_->setSensorConfig(sensor_cfg);
    config_mgr_->setActuatorConfig(actuator_cfg);
    config_mgr_->setDisplayConfig(display_cfg);
    if (doc["control"].is<JsonObject>()) {
        config_mgr_->setObjectConfig("control", doc["control"].as<JsonObject>());
    }
    config_mgr_->save();

    if (hardware_reload_cb_) {
        hardware_reload_cb_();
    }

    sendJson(request, 200, "{\"status\":\"saved\"}");
}

void WebServer::handleNotFound(AsyncWebServerRequest* request) {
    if (wifi_mgr_ && wifi_mgr_->isAPActive()) {
        handleCaptivePortal(request);
        return;
    }
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
    return R"HTML(<!DOCTYPE html>
<html lang="pt">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Philarmony WiFi</title>
<style>
body{font-family:system-ui,sans-serif;margin:0;background:#102018;color:#e8f5e9}
.wrap{max-width:420px;margin:8vh auto;padding:1.5rem}
h1{font-size:1.8rem;margin:0 0 .25rem}
.sub{opacity:.8;margin-bottom:1.5rem}
label{display:block;margin:.75rem 0 .25rem}
input{width:100%;padding:.65rem;border:1px solid #2e7d32;border-radius:6px;background:#0b1510;color:#fff;box-sizing:border-box}
button{margin-top:1.25rem;width:100%;padding:.8rem;border:0;border-radius:6px;background:#43a047;color:#fff;font-weight:600}
.lang{font-size:.85rem;opacity:.7;margin-top:1rem}
</style>
</head>
<body>
<div class="wrap">
<h1>Philarmony</h1>
<p class="sub">Configurar WiFi / Configure WiFi</p>
<form method="POST" action="/api/wifi/config">
<label for="ssid">SSID</label>
<input id="ssid" name="ssid" required maxlength="32" autocomplete="ssid"/>
<label for="password">Senha / Password</label>
<input id="password" name="password" type="password" maxlength="64" autocomplete="current-password"/>
<button type="submit">Salvar e Conectar / Save &amp; Connect</button>
</form>
<p class="lang">PT: Informe a rede local. EN: Enter your local network credentials.</p>
</div>
</body>
</html>)HTML";
}

}  // namespace filament_dryer
