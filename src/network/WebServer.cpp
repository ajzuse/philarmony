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

#include "WebServer.hpp"

#include "../core/ConfigManager.hpp"
#include "../core/LogManager.hpp"
#include "../core/HardwareConfigParser.hpp"
#include "../core/StateMachine.hpp"
#include "WifiManager.hpp"
#include "firmware_version.h"
#include "../plugins/IPlugin.hpp"
#include <WiFi.h>
#include <Esp.h>

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
                      WifiManager* wifi_mgr,
                      StateMachine* state_machine) {
    config_mgr_ = config_mgr;
    log_mgr_ = log_mgr;
    hw_parser_ = hw_parser;
    driver_registry_ = driver_registry;
    wifi_mgr_ = wifi_mgr;
    state_machine_ = state_machine;

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
    server_->on("/api/info", HTTP_GET, [this](AsyncWebServerRequest* request) { handleInfo(request); });
    server_->on(
        "/api/wifi/config", HTTP_POST,
        [this](AsyncWebServerRequest* request) {
            auto* stored = static_cast<String*>(request->_tempObject);
            if (stored) {
                String body = *stored;
                delete stored;
                request->_tempObject = nullptr;

                // Prefer form fields when present; otherwise treat body as JSON
                if (request->hasParam("ssid", true) || request->hasParam("password", true)) {
                    handleWifiConfigPost(request);
                } else if (!body.isEmpty()) {
                    JsonDocument doc;
                    if (deserializeJson(doc, body) != DeserializationError::Ok) {
                        sendError(request, 400, "Invalid JSON body");
                        return;
                    }
                    const String ssid = doc["ssid"] | "";
                    const String password = doc["password"] | "";
                    handleWifiConfigApply(request, ssid, password);
                } else {
                    handleWifiConfigPost(request);
                }
                return;
            }
            handleWifiConfigPost(request);
        },
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            if (index == 0) {
                auto* body = new String();
                body->reserve(total);
                request->_tempObject = body;
            }
            auto* body = static_cast<String*>(request->_tempObject);
            if (!body) {
                return;
            }
            for (size_t i = 0; i < len; ++i) {
                *body += static_cast<char>(data[i]);
            }
        });
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
    request->redirect("/");
}

void WebServer::handleRoot(AsyncWebServerRequest* request) {
    request->send(200, "text/html", getConfigPageHTML());
}

void WebServer::handleInfo(AsyncWebServerRequest* request) {
    JsonDocument doc;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["firmware_name"] = FIRMWARE_NAME;
#if defined(ESP_IDF_VERSION)
    doc["chip_model"] = ESP.getChipModel();
    doc["mac_address"] = WiFi.macAddress();
    doc["free_heap_bytes"] = ESP.getFreeHeap();
#else
    doc["chip_model"] = "native";
    doc["mac_address"] = "00:00:00:00:00:00";
    doc["free_heap_bytes"] = 0;
#endif
    if (wifi_mgr_) {
        if (wifi_mgr_->isConnected()) doc["system_status"] = "ready";
        else if (wifi_mgr_->isAPActive()) doc["system_status"] = "hotspot";
        else if (wifi_mgr_->isConnecting()) doc["system_status"] = "connecting";
        else doc["system_status"] = "boot";
    } else {
        doc["system_status"] = "unknown";
    }
    doc["active_feature"] = "001-filament-dryer-esp32";
    String json;
    serializeJson(doc, json);
    sendJson(request, 200, json);
}

void WebServer::handleWifiConfigPost(AsyncWebServerRequest* request) {
    String ssid;
    String password;
    if (request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    }
    handleWifiConfigApply(request, ssid, password);
}

void WebServer::handleWifiConfigApply(AsyncWebServerRequest* request, const String& ssid,
                                      const String& password) {
    if (!config_mgr_) {
        sendError(request, 500, "Config manager unavailable");
        return;
    }

    WifiConfig config = config_mgr_->getWifiConfig();
    config.ssid = ssid;
    config.password = password;
    config.valid = !config.ssid.isEmpty();

    if (!config_mgr_->setWifiConfig(config)) {
        sendError(request, 500, "Failed to persist WiFi configuration");
        return;
    }

    if (wifi_mgr_) {
        wifi_mgr_->setConfig(config);
    }

    sendJson(request, 200,
             "{\"status\":\"success\",\"message\":\"Credenciais salvas. Reiniciando conexao...\"}");
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

    const char* filename = drying_log ? "drying.log" : "system.log";
    AsyncWebServerResponse* response =
        request->beginResponse(200, "text/plain; charset=utf-8", body);
    response->addHeader("Content-Disposition",
                        String("attachment; filename=\"") + filename + "\"");
    request->send(response);
}

void WebServer::buildKlipperHardwareConfig(JsonObject& root) {
    const SensorConfig sensor = config_mgr_->getSensorConfig();
    const ActuatorConfig actuator = config_mgr_->getActuatorConfig();
    const DisplayConfig display = config_mgr_->getDisplayConfig();

    JsonArray sensors = root["sensors"].to<JsonArray>();

    if (sensor.is_integrated) {
        JsonObject primary = sensors.add<JsonObject>();
        primary["id"] = "chamber_temp";
        primary["type"] = sensor.type;
        JsonArray caps = primary["capabilities"].to<JsonArray>();
        caps.add("temperature");
        caps.add("humidity");
        JsonObject bus = primary["bus"].to<JsonObject>();
        if (sensor.gpio_pin >= 0 &&
            (sensor.type.indexOf("dht") >= 0 || sensor.type.indexOf("ds18") >= 0 ||
             sensor.type == "ntc" || sensor.type == "thermistor" || sensor.type == "am2302")) {
            bus["type"] = "onewire";
            bus["pin"] = sensor.gpio_pin;
        } else {
            bus["type"] = "i2c";
            bus["bus"] = sensor.i2c_bus;
            bus["address"] = sensor.i2c_address;
            bus["sda_pin"] = sensor.sda_pin;
            bus["scl_pin"] = sensor.scl_pin;
        }
        JsonObject cal = primary["calibration"].to<JsonObject>();
        cal["temperature_offset"] = sensor.temperature_offset;
        cal["temperature_scale"] = sensor.temperature_scale;
        cal["humidity_offset"] = sensor.humidity_offset;
        cal["humidity_scale"] = sensor.humidity_scale;
    } else {
        JsonObject temp = sensors.add<JsonObject>();
        temp["id"] = "chamber_temp";
        temp["type"] = sensor.type;
        JsonArray temp_caps = temp["capabilities"].to<JsonArray>();
        temp_caps.add("temperature");
        JsonObject temp_bus = temp["bus"].to<JsonObject>();
        if (sensor.gpio_pin >= 0) {
            temp_bus["type"] = "onewire";
            temp_bus["pin"] = sensor.gpio_pin;
        } else {
            temp_bus["type"] = "i2c";
            temp_bus["bus"] = sensor.i2c_bus;
            temp_bus["address"] = sensor.i2c_address;
            temp_bus["sda_pin"] = sensor.sda_pin;
            temp_bus["scl_pin"] = sensor.scl_pin;
        }
        JsonObject temp_cal = temp["calibration"].to<JsonObject>();
        temp_cal["temperature_offset"] = sensor.temperature_offset;
        temp_cal["temperature_scale"] = sensor.temperature_scale;

        if (!sensor.humidity_type.isEmpty()) {
            JsonObject hum = sensors.add<JsonObject>();
            hum["id"] = "chamber_humidity";
            hum["type"] = sensor.humidity_type;
            JsonArray hum_caps = hum["capabilities"].to<JsonArray>();
            hum_caps.add("humidity");
            JsonObject hum_bus = hum["bus"].to<JsonObject>();
            if (sensor.humidity_gpio_pin >= 0) {
                hum_bus["type"] = "onewire";
                hum_bus["pin"] = sensor.humidity_gpio_pin;
            } else {
                hum_bus["type"] = "i2c";
                hum_bus["bus"] = sensor.i2c_bus;
                hum_bus["address"] = sensor.humidity_i2c_address;
                hum_bus["sda_pin"] = sensor.humidity_sda_pin;
                hum_bus["scl_pin"] = sensor.humidity_scl_pin;
            }
            JsonObject hum_cal = hum["calibration"].to<JsonObject>();
            hum_cal["humidity_offset"] = sensor.humidity_offset;
            hum_cal["humidity_scale"] = sensor.humidity_scale;
        }
    }

    if (!sensor.extra_temp_type.isEmpty()) {
        JsonObject extra = sensors.add<JsonObject>();
        extra["id"] = "external_temp";
        extra["type"] = sensor.extra_temp_type;
        JsonArray caps = extra["capabilities"].to<JsonArray>();
        caps.add("temperature");
        JsonObject bus = extra["bus"].to<JsonObject>();
        if (sensor.extra_temp_gpio_pin >= 0) {
            bus["type"] = "onewire";
            bus["pin"] = sensor.extra_temp_gpio_pin;
        } else {
            bus["type"] = "i2c";
            bus["bus"] = sensor.i2c_bus;
            bus["address"] = sensor.extra_temp_i2c_address;
            bus["sda_pin"] = sensor.sda_pin;
            bus["scl_pin"] = sensor.scl_pin;
        }
    }

    JsonArray actuators = root["actuators"].to<JsonArray>();
    {
        JsonObject heater = actuators.add<JsonObject>();
        heater["id"] = "heater";
        heater["type"] = actuator.heater_type;
        heater["role"] = "heater";
        JsonObject pins = heater["pins"].to<JsonObject>();
        pins["pwm"] = actuator.heater_pin;
        JsonObject control = heater["control"].to<JsonObject>();
        control["pwm_freq_hz"] = actuator.heater_pwm_freq;
        control["max_power_pct"] = actuator.heater_max_power_pct;
        JsonObject safety = heater["safety_limits"].to<JsonObject>();
        safety["max_power_pct"] = actuator.heater_max_power_pct;
    }
    {
        JsonObject fan = actuators.add<JsonObject>();
        fan["id"] = "exhaust_fan";
        fan["type"] = actuator.fan_type;
        fan["role"] = "fan";
        JsonObject pins = fan["pins"].to<JsonObject>();
        pins["pwm"] = actuator.fan_pin;
        JsonObject control = fan["control"].to<JsonObject>();
        control["pwm_freq_hz"] = actuator.fan_pwm_freq;
        control["cooldown_sec"] = actuator.cooldown_duration_sec;
        JsonObject safety = fan["safety_limits"].to<JsonObject>();
        safety["max_power_pct"] = 100;
    }
    if (actuator.has_custom) {
        JsonObject custom = actuators.add<JsonObject>();
        custom["id"] = "custom";
        custom["type"] = actuator.custom_type;
        custom["role"] = "custom";
        JsonObject pins = custom["pins"].to<JsonObject>();
        pins["gpio"] = actuator.custom_pin;
    }

    JsonObject display_obj = root["display"].to<JsonObject>();
    display_obj["enabled"] = display.enabled;
    display_obj["driver"] = display.driver;
    JsonObject bus = display_obj["bus"].to<JsonObject>();
    bus["type"] = display.bus_type;
    if (display.bus_type == "spi" || display.spi_mosi >= 0) {
        bus["mosi"] = display.spi_mosi;
        bus["sclk"] = display.spi_sclk;
        bus["cs"] = display.spi_cs;
        bus["dc"] = display.dc_pin;
        bus["rst"] = display.rst_pin;
        bus["bl"] = display.backlight_pin;
    }
    JsonObject geometry = display_obj["geometry"].to<JsonObject>();
    geometry["width"] = display.width;
    geometry["height"] = display.height;
    geometry["rotation"] = display.rotation;
    JsonObject layout = display_obj["layout"].to<JsonObject>();
    JsonArray fields = layout["fields"].to<JsonArray>();
    for (const auto& field : display.fields) {
        fields.add(field);
    }
    layout["font_scaling"] = "auto";
    layout["refresh_rate_hz"] = display.refresh_rate_hz;

    root["control"] = config_mgr_->getObjectConfig("control");
}

void WebServer::handleHardwareConfigGet(AsyncWebServerRequest* request) {
    if (!config_mgr_) {
        sendError(request, 500, "Config manager unavailable");
        return;
    }

    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    buildKlipperHardwareConfig(root);

    String json;
    serializeJson(doc, json);
    sendJson(request, 200, json);
}

void WebServer::handleHardwareConfigPostBody(AsyncWebServerRequest* request, const String& body) {
    if (!config_mgr_ || !hw_parser_) {
        sendError(request, 500, "Hardware parser unavailable");
        return;
    }

    if (state_machine_ && (state_machine_->isDrying() || state_machine_->isCoolingDown())) {
        sendError(request, 409, "Cannot reload hardware while drying or cooling down");
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
    if (plugin_mgr_) {
        JsonDocument paramsDoc;
        JsonObject params = paramsDoc.to<JsonObject>();
        String response;
        if (plugin_mgr_->callHttpRequest(request->url(), params, response)) {
            request->send(200, "application/json", response);
            return;
        }
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
