/**
 * WebServer - Implementation
 */
#include "WebServer.hpp"
#include "ConfigManager.hpp"
#include "LogManager.hpp"
#include "HardwareConfigParser.hpp"
#include "DriverRegistry.hpp"

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
    
    server_ = new AsyncWebServer(port_);
    
    setupRoutes();
    
    server_->begin();
    
    Serial.printf("[WebServer] Started on port %d\n", port_);
    return true;
}

void WebServer::setupRoutes() {
    // Captive portal detection (Android, iOS, Windows)
    server_->on("/generate_204", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    server_->on("/fwlink", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    server_->on("/hotspot-detect.html", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    server_->on("/ncsi.txt", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    server_->on("/connecttest.txt", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    
    // Main routes
    server_->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleRoot(request);
    });
    
    server_->on("/info", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleInfo(request);
    });
    
    server_->on("/api/wifi/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleWifiConfigPost(request);
    });
    
    // Log download endpoints
    server_->on("/log/drying", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleLogDownload(request, true);
    });
    
    server_->on("/log/system", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleLogDownload(request, false);
    });
    
    // T019b: HTTP GET/POST /api/hardware/config endpoints
    server_->on("/api/hardware/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleHardwareConfigGet(request);
    });
    
    // POST: use a body handler to receive JSON body
    server_->on("/api/hardware/config", HTTP_POST,
        [this](AsyncWebServerRequest* request) {
            // Final handler - body has been received and processed by body handler
            // The response was already sent in the body handler
        },
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            // Body handler
            static String body_buffer;
            if (index == 0) body_buffer = "";
            for (size_t i = 0; i < len; i++) {
                body_buffer += (char)data[i];
            }
            if (index + len == total) {
                handleHardwareConfigPostBody(request, body_buffer);
                body_buffer = "";
            }
        }
    );
    
    // 404 handler
    server_->onNotFound([this](AsyncWebServerRequest* request) {
        handleNotFound(request);
    });
}

void WebServer::handleCaptivePortal(AsyncWebServerRequest* request) {
    request->redirect("/");
}

void WebServer::handleRoot(AsyncWebServerRequest* request) {
    request->send(200, "text/html", getCaptivePortalHTML());
}

void WebServer::handleInfo(AsyncWebServerRequest* request) {
    if (!config_mgr_) {
        request->send(500, "application/json", "{\"error\":\"Config manager not available\"}");
        return;
    }
    
    StaticJsonDocument<1024> doc;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["chip_model"] = ESP.getChipModel();
    doc["chip_revision"] = ESP.getChipRevision();
    doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
    doc["mac_address"] = WiFi.macAddress();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["heap_size"] = ESP.getHeapSize();
    doc["system_status"] = "ready";
    doc["active_feature"] = "001-filament-dryer-esp32";
    
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

void WebServer::handleWifiConfigPost(AsyncWebServerRequest* request) {
    if (!config_mgr_) {
        request->send(500, "application/json", "{\"error\":\"Config manager not available\"}");
        return;
    }
    
    String ssid, password;
    
    if (request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    }
    
    if (ssid.isEmpty()) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"SSID vazio\"}");
        return;
    }
    
    ConfigManager::WifiConfig config;
    config.ssid = ssid;
    config.password = password;
    config.valid = true;
    
    if (config_mgr_->setWifiConfig(config)) {
        String json = "{\"status\":\"success\",\"message\":\"Credenciais salvas. Reiniciando conexao...\"}";
        request->send(200, "application/json", json);
        
        // Restart WiFi with new config after short delay
        // Note: In practice, this would be handled by the WifiManager
    } else {
        request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Falha ao salvar\"}");
    }
}

void WebServer::handleLogDownload(AsyncWebServerRequest* request, bool drying_log) {
    if (!log_mgr_) {
        request->send(500, "application/json", "{\"error\":\"Log manager not available\"}");
        return;
    }
    
    String log_content;
    bool ok = drying_log ? log_mgr_->getDryingLog(log_content) : log_mgr_->getSystemLog(log_content);
    
    if (!ok) {
        request->send(404, "application/json", "{\"error\":\"Log nao encontrado\"}");
        return;
    }
    
    String filename = drying_log ? "drying.log" : "system.log";
    
    AsyncWebServerResponse* response = request->beginResponse(
        "text/plain", 
        log_content,
        [filename](AsyncWebServerRequest* req, const String& data) {
            // Headers are set in the response
        }
    );
    
    response->addHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    response->addHeader("Cache-Control", "no-cache");
    
    request->send(response);
}

void WebServer::handleNotFound(AsyncWebServerRequest* request) {
    // If captive portal check, redirect to root
    if (request->url().indexOf("generate_204") >= 0 ||
        request->url().indexOf("fwlink") >= 0 ||
        request->url().indexOf("hotspot") >= 0 ||
        request->url().indexOf("ncsi") >= 0 ||
        request->url().indexOf("connecttest") >= 0) {
        request->redirect("/");
        return;
    }
    
    request->send(404, "text/plain", "Not found");
}

// T019b: GET /api/hardware/config - returns current hardware config as JSON
void WebServer::handleHardwareConfigGet(AsyncWebServerRequest* request) {
    if (!config_mgr_) {
        sendError(request, 500, "Config manager not available");
        return;
    }
    
    JsonDocument doc;
    
    // Sensors (generic array)
    JsonObject saved_sensors = config_mgr_->getObjectConfig("sensors");
    if (!saved_sensors.isNull() && saved_sensors.is<JsonArray>()) {
        JsonArray arr = saved_sensors.as<JsonArray>();
        JsonArray out = doc.createNestedArray("sensors");
        for (JsonVariant v : arr) {
            out.add(v);
        }
    } else {
        // Fall back to legacy single-sensor config
        JsonObject sensors_obj = doc.createNestedObject("sensors");
        SensorConfig sc = config_mgr_->getSensorConfig();
        sensors_obj["chamber_temp"]["type"] = sc.type;
        sensors_obj["chamber_temp"]["i2c_address"] = sc.i2c_address;
        sensors_obj["chamber_temp"]["sda_pin"] = sc.sda_pin;
        sensors_obj["chamber_temp"]["scl_pin"] = sc.scl_pin;
    }
    
    // Actuators (generic array)
    JsonObject saved_actuators = config_mgr_->getObjectConfig("actuators");
    if (!saved_actuators.isNull() && saved_actuators.is<JsonArray>()) {
        JsonArray arr = saved_actuators.as<JsonArray>();
        JsonArray out = doc.createNestedArray("actuators");
        for (JsonVariant v : arr) {
            out.add(v);
        }
    } else {
        // Fall back to legacy actuator config
        JsonObject acts_obj = doc.createNestedObject("actuators");
        ActuatorConfig ac = config_mgr_->getActuatorConfig();
        JsonObject heater = acts_obj.createNestedObject("heater");
        heater["type"] = "mosfet_pwm";
        heater["role"] = "heater";
        heater["pins"]["pwm"] = ac.heater_pin;
        JsonObject fan = acts_obj.createNestedObject("exhaust_fan");
        fan["type"] = "fan_pwm";
        fan["role"] = "fan";
        fan["pins"]["pwm"] = ac.fan_pin;
    }
    
    // Display
    JsonObject display = doc.createNestedObject("display");
    DisplayConfig dc = config_mgr_->getDisplayConfig();
    display["enabled"] = dc.enabled;
    display["driver"] = dc.driver;
    display["bus_type"] = dc.bus_type;
    display["width"] = dc.width;
    display["height"] = dc.height;
    display["rotation"] = dc.rotation;
    display["spi_mosi"] = dc.spi_mosi;
    display["spi_sclk"] = dc.spi_sclk;
    display["spi_cs"] = dc.spi_cs;
    display["dc_pin"] = dc.dc_pin;
    display["rst_pin"] = dc.rst_pin;
    display["backlight_pin"] = dc.backlight_pin;
    JsonArray fields_out = display.createNestedArray("fields");
    for (const String& f : dc.fields) {
        fields_out.add(f);
    }
    
    // Control algorithm
    JsonObject control = doc.createNestedObject("control");
    JsonObject saved_control = config_mgr_->getObjectConfig("control");
    if (!saved_control.isNull()) {
        control["algorithm"] = saved_control["algorithm"] | "pid";
        control["auto_tune"] = saved_control["auto_tune"] | false;
        if (saved_control.containsKey("parameters")) {
            JsonObject params_in = saved_control["parameters"].as<JsonObject>();
            JsonObject params_out = control.createNestedObject("parameters");
            for (JsonPair kv : params_in) {
                params_out[kv.key()] = kv.value();
            }
        }
        if (saved_control.containsKey("safety_limits")) {
            JsonObject sl = saved_control["safety_limits"].as<JsonObject>();
            JsonObject sl_out = control.createNestedObject("safety_limits");
            for (JsonPair kv : sl) {
                sl_out[kv.key()] = kv.value();
            }
        }
    } else {
        control["algorithm"] = "pid";
        control["auto_tune"] = false;
    }
    
    // Available drivers (informational)
    if (driver_registry_) {
        JsonObject available = doc.createNestedObject("available_drivers");
        JsonArray sensors_arr = available.createNestedArray("sensors");
        for (const String& s : driver_registry_->listSensors()) sensors_arr.add(s);
        JsonArray actuators_arr = available.createNestedArray("actuators");
        for (const String& s : driver_registry_->listActuators()) actuators_arr.add(s);
        JsonArray displays_arr = available.createNestedArray("displays");
        for (const String& s : driver_registry_->listDisplays()) displays_arr.add(s);
        JsonArray controls_arr = available.createNestedArray("controls");
        for (const String& s : driver_registry_->listControls()) controls_arr.add(s);
    }
    
    String json;
    serializeJson(doc, json);
    sendJson(request, 200, json);
}

// T019b: POST /api/hardware/config - accept hardware config JSON
void WebServer::handleHardwareConfigPostBody(AsyncWebServerRequest* request, const String& body) {
    if (!config_mgr_) {
        sendError(request, 500, "Config manager not available");
        return;
    }
    
    if (body.isEmpty()) {
        sendError(request, 400, "Empty request body");
        return;
    }
    
    // Parse JSON
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        sendError(request, 400, "JSON parse error: " + String(err.c_str()));
        return;
    }
    
    JsonObject payload = doc.as<JsonObject>();
    
    // Validate using HardwareConfigParser if available
    if (hw_parser_) {
        SensorConfig sc = config_mgr_->getSensorConfig();
        ActuatorConfig ac = config_mgr_->getActuatorConfig();
        DisplayConfig dc = config_mgr_->getDisplayConfig();
        ControlConfig cc;
        
        ControlConfig current_control;
        JsonObject existing_control = config_mgr_->getObjectConfig("control");
        if (!existing_control.isNull()) {
            current_control.algorithm = existing_control["algorithm"] | "pid";
            current_control.auto_tune = existing_control["auto_tune"] | false;
            if (existing_control.containsKey("safety_limits")) {
                JsonObject sl = existing_control["safety_limits"];
                current_control.safety_limits.hard_temp_limit_c = sl["hard_temp_limit_c"] | 80.0f;
                current_control.safety_limits.max_heater_power_pct = sl["max_heater_power_pct"] | 100;
                current_control.safety_limits.sensor_timeout_ms = sl["sensor_timeout_ms"] | 600;
                current_control.safety_limits.thermal_runaway_time_sec = sl["thermal_runaway_time_sec"] | 45;
                current_control.safety_limits.thermal_runaway_temp_rise_c = sl["thermal_runaway_temp_rise_c"] | 0.5f;
            }
        }
        cc = current_control;
        
        auto validation = hw_parser_->parse(payload, sc, ac, dc, cc);
        if (!validation.valid) {
            String err_msg = "Schema validation failed:";
            for (const auto& e : validation.errors) {
                err_msg += " " + e + ";";
            }
            sendError(request, 400, err_msg);
            return;
        }
    }
    
    // Validate driver types via DriverRegistry
    if (driver_registry_) {
        if (payload.containsKey("sensors")) {
            JsonArray sensors = payload["sensors"].as<JsonArray>();
            for (JsonVariant v : sensors) {
                JsonObject sensor = v.as<JsonObject>();
                String type = sensor["type"] | "";
                if (!type.isEmpty() && !driver_registry_->hasSensor(type)) {
                    sendError(request, 400, "Unknown sensor driver: " + type);
                    return;
                }
            }
        }
        if (payload.containsKey("actuators")) {
            JsonArray actuators = payload["actuators"].as<JsonArray>();
            for (JsonVariant v : actuators) {
                JsonObject actuator = v.as<JsonObject>();
                String type = actuator["type"] | "";
                if (!type.isEmpty() && !driver_registry_->hasActuator(type)) {
                    sendError(request, 400, "Unknown actuator driver: " + type);
                    return;
                }
            }
        }
        if (payload.containsKey("display")) {
            JsonObject display = payload["display"].as<JsonObject>();
            String drv = display["driver"] | "auto";
            if (drv != "auto" && !drv.isEmpty() && !driver_registry_->hasDisplay(drv)) {
                sendError(request, 400, "Unknown display driver: " + drv);
                return;
            }
        }
        if (payload.containsKey("control")) {
            JsonObject control = payload["control"].as<JsonObject>();
            String algo = control["algorithm"] | "pid";
            if (!driver_registry_->hasControl(algo)) {
                sendError(request, 400, "Unknown control algorithm: " + algo);
                return;
            }
        }
    }
    
    // Persist config (same logic as WebSocket handler)
    
    // Sensors
    if (payload.containsKey("sensors")) {
        JsonArray sensors = payload["sensors"].as<JsonArray>();
        if (sensors.size() > 0) {
            JsonObject first_sensor = sensors[0].as<JsonObject>();
            SensorConfig sc = config_mgr_->getSensorConfig();
            sc.type = first_sensor["type"] | sc.type;
            if (first_sensor.containsKey("bus")) {
                JsonObject bus = first_sensor["bus"].as<JsonObject>();
                String bus_type = bus["type"] | "i2c";
                if (bus_type == "i2c") {
                    sc.sda_pin = bus["sda_pin"] | sc.sda_pin;
                    sc.scl_pin = bus["scl_pin"] | sc.scl_pin;
                    sc.i2c_address = bus["address"] | sc.i2c_address;
                } else {
                    sc.gpio_pin = bus["pin"] | sc.gpio_pin;
                }
            }
            config_mgr_->setSensorConfig(sc);
        }
        
        JsonDocument sensor_doc;
        sensor_doc.set(sensors);
        JsonObject sensor_obj = sensor_doc.as<JsonObject>();
        config_mgr_->setObjectConfig("sensors", sensor_obj);
    }
    
    // Actuators
    if (payload.containsKey("actuators")) {
        JsonArray actuators = payload["actuators"].as<JsonArray>();
        ActuatorConfig ac = config_mgr_->getActuatorConfig();
        
        for (JsonVariant v : actuators) {
            JsonObject actuator = v.as<JsonObject>();
            String role = actuator["role"] | "";
            
            if (role == "heater" && actuator.containsKey("pins")) {
                JsonObject pins = actuator["pins"].as<JsonObject>();
                ac.heater_pin = pins["pwm"] | ac.heater_pin;
            } else if (role == "fan" && actuator.containsKey("pins")) {
                JsonObject pins = actuator["pins"].as<JsonObject>();
                ac.fan_pin = pins["pwm"] | ac.fan_pin;
            }
        }
        config_mgr_->setActuatorConfig(ac);
        
        JsonDocument act_doc;
        act_doc.set(actuators);
        JsonObject act_obj = act_doc.as<JsonObject>();
        config_mgr_->setObjectConfig("actuators", act_obj);
    }
    
    // Display
    if (payload.containsKey("display")) {
        JsonObject display = payload["display"].as<JsonObject>();
        DisplayConfig dc = config_mgr_->getDisplayConfig();
        dc.enabled = display["enabled"] | dc.enabled;
        dc.driver = display["driver"] | dc.driver;
        dc.bus_type = display["bus_type"] | dc.bus_type;
        dc.width = display["width"] | dc.width;
        dc.height = display["height"] | dc.height;
        dc.rotation = display["rotation"] | dc.rotation;
        dc.spi_mosi = display["spi_mosi"] | dc.spi_mosi;
        dc.spi_sclk = display["spi_sclk"] | dc.spi_sclk;
        dc.spi_cs = display["spi_cs"] | dc.spi_cs;
        dc.dc_pin = display["dc_pin"] | dc.dc_pin;
        dc.rst_pin = display["rst_pin"] | dc.rst_pin;
        dc.backlight_pin = display["backlight_pin"] | dc.backlight_pin;
        if (display.containsKey("fields")) {
            JsonArray arr = display["fields"].as<JsonArray>();
            dc.fields.clear();
            for (JsonVariant v : arr) {
                dc.fields.push_back(v.as<String>());
            }
        }
        config_mgr_->setDisplayConfig(dc);
        
        JsonDocument disp_doc;
        disp_doc.set(display);
        JsonObject disp_obj = disp_doc.as<JsonObject>();
        config_mgr_->setObjectConfig("display", disp_obj);
    }
    
    // Control
    if (payload.containsKey("control")) {
        JsonObject control = payload["control"].as<JsonObject>();
        
        JsonDocument ctrl_doc;
        ctrl_doc.set(control);
        JsonObject ctrl_obj = ctrl_doc.as<JsonObject>();
        config_mgr_->setObjectConfig("control", ctrl_obj);
        
        String algo = control["algorithm"] | "pid";
        if (algo == "pid" && control.containsKey("parameters")) {
            JsonObject params = control["parameters"].as<JsonObject>();
            PidConfig pid;
            pid.kp = params["kp"] | 0.0f;
            pid.ki = params["ki"] | 0.0f;
            pid.kd = params["kd"] | 0.0f;
            pid.calibrated = true;
            config_mgr_->setPidConfig(pid);
        }
    }
    
    sendJson(request, 200, "{\"status\":\"saved\"}");
}

// T019b: HTTP JSON helpers
void WebServer::sendJson(AsyncWebServerRequest* request, int code, const String& json) {
    AsyncWebServerResponse* response = request->beginResponse(code, "application/json", json);
    response->addHeader("Cache-Control", "no-cache");
    request->send(response);
}

void WebServer::sendError(AsyncWebServerRequest* request, int code, const String& error) {
    StaticJsonDocument<256> doc;
    doc["status"] = "error";
    doc["error"] = error;
    String json;
    serializeJson(doc, json);
    sendJson(request, code, json);
}

const char* WebServer::getCaptivePortalHTML() {
    return R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Filament Dryer - Configuração WiFi</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
            min-height: 100vh; display: flex; align-items: center; justify-content: center;
            padding: 20px;
        }
        .container { 
            background: white; border-radius: 16px; padding: 40px; 
            max-width: 400px; width: 100%; box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        .logo { text-align: center; margin-bottom: 30px; }
        .logo svg { width: 80px; height: 80px; }
        h1 { color: #1e3c72; text-align: center; margin-bottom: 10px; font-size: 24px; }
        .subtitle { color: #666; text-align: center; margin-bottom: 30px; font-size: 14px; }
        .form-group { margin-bottom: 20px; }
        label { display: block; margin-bottom: 8px; color: #333; font-weight: 500; font-size: 14px; }
        input { 
            width: 100%; padding: 14px 16px; border: 2px solid #e0e0e0; 
            border-radius: 8px; font-size: 16px; transition: border-color 0.2s;
        }
        input:focus { outline: none; border-color: #1e3c72; }
        .btn { 
            width: 100%; padding: 16px; background: #1e3c72; color: white; 
            border: none; border-radius: 8px; font-size: 16px; font-weight: 600;
            cursor: pointer; transition: background 0.2s;
        }
        .btn:hover { background: #2a5298; }
        .btn:disabled { background: #999; cursor: not-allowed; }
        .info { 
            background: #f0f4f8; border-radius: 8px; padding: 16px; 
            margin-top: 20px; font-size: 13px; color: #555;
        }
        .info strong { color: #1e3c72; }
        .spinner { display: none; width: 20px; height: 20px; border: 3px solid #f3f3f3; 
            border-top: 3px solid #1e3c72; border-radius: 50%; animation: spin 1s linear infinite; 
            margin: 0 auto; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
        .success { display: none; text-align: center; color: #27ae60; }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">
            <svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg">
                <circle cx="50" cy="50" r="45" fill="none" stroke="#1e3c72" stroke-width="4"/>
                <path d="M30 50 Q50 30 70 50 Q50 70 30 50" fill="none" stroke="#1e3c72" stroke-width="3"/>
                <circle cx="50" cy="50" r="15" fill="#1e3c72"/>
            </svg>
        </div>
        <h1>Filament Dryer ESP32</h1>
        <p class="subtitle">Configure a conexão WiFi para iniciar</p>
        
        <form id="wifiForm">
            <div class="form-group">
                <label for="ssid">Nome da Rede (SSID)</label>
                <input type="text" id="ssid" name="ssid" required autocomplete="off" placeholder="MinhaRedeWiFi">
            </div>
            <div class="form-group">
                <label for="password">Senha</label>
                <input type="password" id="password" name="password" autocomplete="new-password" placeholder="Senha da rede">
            </div>
            <button type="submit" class="btn" id="submitBtn">Conectar</button>
            <div class="spinner" id="spinner"></div>
        </form>
        
        <div class="success" id="successMsg">
            ✓ Credenciais salvas! O dispositivo vai reiniciar a conexão...
        </div>
        
        <div class="info">
            <strong>Informações:</strong>
            <ul style="margin: 10px 0 0 20px; padding: 0;">
                <li>AP padrão: <strong>philarmony</strong> / <strong>philarmony</strong></li>
                <li>IP do AP: <strong>192.168.4.1</strong></li>
                <li>Após conectar, acesse via WebSocket no IP do dispositivo</li>
            </ul>
        </div>
    </div>
    
    <script>
        document.getElementById('wifiForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            const btn = document.getElementById('submitBtn');
            const spinner = document.getElementById('spinner');
            const success = document.getElementById('successMsg');
            
            btn.disabled = true;
            btn.textContent = '';
            spinner.style.display = 'block';
            
            const formData = new FormData(e.target);
            
            try {
                const response = await fetch('/api/wifi/config', {
                    method: 'POST',
                    body: formData
                });
                
                const result = await response.json();
                
                if (result.status === 'success') {
                    spinner.style.display = 'none';
                    success.style.display = 'block';
                    btn.style.display = 'none';
                } else {
                    throw new Error(result.message || 'Erro desconhecido');
                }
            } catch (err) {
                alert('Erro: ' + err.message);
                btn.disabled = false;
                btn.textContent = 'Conectar';
                spinner.style.display = 'none';
            }
        });
    </script>
</body>
</html>
)rawliteral";
}