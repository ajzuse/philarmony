/**
 * WebSocketServer - Implementation
 */
#include "WebSocketServer.hpp"
#include "ConfigManager.hpp"
#include "StateMachine.hpp"
#include "SafetyEngine.hpp"
#include "LogManager.hpp"
#include "HardwareConfigParser.hpp"
#include "DriverRegistry.hpp"
#include "drivers/interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

WebSocketServer::WebSocketServer(uint16_t port, const char* path) 
    : port_(port), path_(path) {}

WebSocketServer::~WebSocketServer() {
    if (ws_) {
        ws_->closeAll();
        delete ws_;
    }
}

bool WebSocketServer::begin(ConfigManager* config_mgr, StateMachine* state_machine,
                            SafetyEngine* safety, LogManager* log_mgr,
                            PidAutotuneController* pid_autotune,
                            HardwareConfigParser* hw_parser,
                            DriverRegistry* driver_registry) {
    config_mgr_ = config_mgr;
    state_machine_ = state_machine;
    safety_ = safety;
    log_mgr_ = log_mgr;
    pid_autotune_ = pid_autotune;
    hw_parser_ = hw_parser;
    driver_registry_ = driver_registry;
    
    ws_ = new AsyncWebSocket(path_);
    ws_->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                        AwsEventType type, void* arg, uint8_t* data, size_t len) {
        onEvent(server, client, type, arg, data, len);
    });
    
    Serial.printf("[WebSocketServer] Started on port %d, path %s\n", port_, path_);
    return true;
}

void WebSocketServer::loop() {
    if (ws_) {
        ws_->cleanupClients();
    }
}

void WebSocketServer::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                              AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("[WebSocketServer] Client #%u connected from %s\n", 
                          client->id(), client->remoteIP().toString().c_str());
            clients_.push_back({client->id(), false, false});
            break;
            
        case WS_EVT_DISCONNECT:
            Serial.printf("[WebSocketServer] Client #%u disconnected\n", client->id());
            clients_.erase(
                std::remove_if(clients_.begin(), clients_.end(),
                    [client](const ClientInfo& c) { return c.id == client->id(); }),
                clients_.end()
            );
            break;
            
        case WS_EVT_DATA: {
            AwsFrameInfo* info = reinterpret_cast<AwsFrameInfo*>(arg);
            if (info->final && info->index == 0 && info->len == len 
                && info->opcode == WS_TEXT) {
                handleMessage(client, data, len);
            }
            break;
        }
            
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void WebSocketServer::handleMessage(AsyncWebSocketClient* client, uint8_t* data, size_t len) {
    // Null-terminate the incoming text frame
    data[len] = '\0';

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, reinterpret_cast<char*>(data));
    if (err) {
        sendError(client, "error", String("JSON parse error: ") + err.c_str());
        return;
    }

    dispatchTopic(client, doc.as<JsonObject>());
}

void WebSocketServer::dispatchTopic(AsyncWebSocketClient* client, const JsonObject& doc) {
    String topic = doc["topic"] | "";
    JsonObject payload = doc["payload"].as<JsonObject>();
    
    if (topic.isEmpty()) {
        sendError(client, "error", "Missing topic");
        return;
    }
    
    if (topic == "control/start") {
        handleControlStart(client, payload);
    } else if (topic == "control/stop") {
        handleControlStop(client, payload);
    } else if (topic == "config/hardware") {
        handleConfigHardware(client, payload);
    } else if (topic == "config/display") {
        handleConfigDisplay(client, payload);
    } else if (topic == "config/profiles/list"
            || topic == "config/profiles/get"
            || topic == "config/profiles/create"
            || topic == "config/profiles/update"
            || topic == "config/profiles/delete"
            || topic == "config/profiles/reset_defaults") {
        // Inject action from topic suffix for unified handler
        StaticJsonDocument<256> action_doc;
        action_doc.set(payload);
        String action = topic.substring(topic.lastIndexOf('/') + 1);
        action_doc["action"] = action;
        handleConfigProfiles(client, action_doc.as<JsonObject>());
    } else if (topic == "control/pid_calibrate") {
        handlePidCalibrate(client, payload);
    } else if (topic == "status/subscribe") {
        handleStatusSubscribe(client, payload);
    } else if (topic == "status/unsubscribe") {
        handleStatusUnsubscribe(client, payload);
    } else if (topic == "logs/stream/subscribe") {
        handleLogsSubscribe(client, payload);
    } else if (topic == "logs/stream/unsubscribe") {
        handleLogsUnsubscribe(client, payload);
    } else {
        sendError(client, topic, "Unknown topic: " + topic);
    }
}

void WebSocketServer::handleControlStart(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!state_machine_ || !safety_) {
        sendError(client, "control/start", "System not ready");
        return;
    }
    
    DryingSession session;
    session.profile_id = payload["profile_id"] | "";
    session.target_temp_c = payload["target_temp_c"] | 50.0f;
    session.max_duration_min = payload["max_duration_min"] | 240;
    session.target_humidity_pct = payload["target_humidity_pct"] | 0.0f;
    
    if (state_machine_->startDrying(session)) {
        JsonObject response = ws_->makeJsonObject();
        response["status"] = "started";
        response["session_id"] = session.session_id;
        sendResponse(client, "control/start/response", response);
    } else {
        sendError(client, "control/start", "Failed to start drying cycle");
    }
}

void WebSocketServer::handleControlStop(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!state_machine_) {
        sendError(client, "control/stop", "System not ready");
        return;
    }
    
    String reason = payload["reason"] | "user_requested";
    DryingStopReason stop_reason = DryingStopReason::USER_STOPPED;
    
    if (reason == "humidity_reached") stop_reason = DryingStopReason::HUMIDITY_REACHED;
    else if (reason == "max_time") stop_reason = DryingStopReason::MAX_TIME;
    else if (reason == "over_temp") stop_reason = DryingStopReason::SAFETY_CUTOFF;
    else if (reason == "sensor_error") stop_reason = DryingStopReason::SENSOR_ERROR;
    
    state_machine_->stopDrying(stop_reason);
    
    JsonObject response = ws_->makeJsonObject();
    response["status"] = "stopped";
    response["reason"] = reason;
    sendResponse(client, "control/stop/response", response);
}

void WebSocketServer::handleConfigHardware(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!config_mgr_) {
        sendError(client, "config/hardware", "Config manager not available");
        return;
    }
    
    // T019a: Generic hardware config handler with full JSON schema validation
    // Supports the Klipper-style multi-sensor/actuator/display/control payload
    
    // Phase 1: Schema validation (if parser available)
    if (hw_parser_) {
        SensorConfig sensor_cfg = config_mgr_->getSensorConfig();
        ActuatorConfig actuator_cfg = config_mgr_->getActuatorConfig();
        DisplayConfig display_cfg = config_mgr_->getDisplayConfig();
        ControlConfig control_cfg;
        
        // Apply defaults from current control config (algorithm, safety limits)
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
        control_cfg = current_control;
        
        auto validation = hw_parser_->parse(payload, sensor_cfg, actuator_cfg, display_cfg, control_cfg);
        if (!validation.valid) {
            // Build detailed error response
            String err_msg = "Schema validation failed:";
            for (const auto& e : validation.errors) {
                err_msg += " " + e + ";";
            }
            sendError(client, "config/hardware", err_msg);
            return;
        }
        
        // Log warnings (but proceed)
        for (const auto& w : validation.warnings) {
            if (log_mgr_) {
                // Warnings don't abort the save
                Serial.printf("[WebSocketServer] config/hardware warning: %s\n", w.c_str());
            }
        }
    }
    
    // Phase 2: Verify driver types are registered (if registry available)
    if (driver_registry_) {
        // Validate sensor types
        if (payload.containsKey("sensors")) {
            JsonArray sensors = payload["sensors"].as<JsonArray>();
            for (JsonVariant v : sensors) {
                JsonObject sensor = v.as<JsonObject>();
                String type = sensor["type"] | "";
                if (!type.isEmpty() && !driver_registry_->hasSensor(type)) {
                    sendError(client, "config/hardware", 
                        "Unknown sensor driver: " + type);
                    return;
                }
            }
        }
        
        // Validate actuator types
        if (payload.containsKey("actuators")) {
            JsonArray actuators = payload["actuators"].as<JsonArray>();
            for (JsonVariant v : actuators) {
                JsonObject actuator = v.as<JsonObject>();
                String type = actuator["type"] | "";
                if (!type.isEmpty() && !driver_registry_->hasActuator(type)) {
                    sendError(client, "config/hardware", 
                        "Unknown actuator driver: " + type);
                    return;
                }
            }
        }
        
        // Validate display driver
        if (payload.containsKey("display")) {
            JsonObject display = payload["display"].as<JsonObject>();
            String driver = display["driver"] | "auto";
            if (driver != "auto" && !driver.isEmpty() && 
                !driver_registry_->hasDisplay(driver)) {
                sendError(client, "config/hardware", 
                    "Unknown display driver: " + driver);
                return;
            }
        }
        
        // Validate control algorithm
        if (payload.containsKey("control")) {
            JsonObject control = payload["control"].as<JsonObject>();
            String algo = control["algorithm"] | "pid";
            if (!driver_registry_->hasControl(algo)) {
                sendError(client, "config/hardware", 
                    "Unknown control algorithm: " + algo);
                return;
            }
        }
    }
    
    // Phase 3: Persist configuration sections
    
    // Sensors - use first sensor as the primary sensor config
    if (payload.containsKey("sensors")) {
        JsonArray sensors = payload["sensors"].as<JsonArray>();
        if (sensors.size() > 0) {
            JsonObject first_sensor = sensors[0].as<JsonObject>();
            SensorConfig sc = config_mgr_->getSensorConfig();
            sc.type = first_sensor["type"] | sc.type;
            sc.is_integrated = true; // Default; driver-specific
            sc.i2c_bus = 0;
            
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
            
            // Also save the full sensors array as object config (generic)
            JsonDocument sensor_doc;
            sensor_doc.set(sensors);
            JsonObject sensor_obj = sensor_doc.as<JsonObject>();
            config_mgr_->setObjectConfig("sensors", sensor_obj);
        }
    }
    
    // Actuators
    if (payload.containsKey("actuators")) {
        JsonArray actuators = payload["actuators"].as<JsonArray>();
        ActuatorConfig ac = config_mgr_->getActuatorConfig();
        
        for (JsonVariant v : actuators) {
            JsonObject actuator = v.as<JsonObject>();
            String role = actuator["role"] | "";
            String type = actuator["type"] | "";
            
            if (role == "heater") {
                if (actuator.containsKey("pins")) {
                    JsonObject pins = actuator["pins"].as<JsonObject>();
                    ac.heater_pin = pins["pwm"] | ac.heater_pin;
                }
                if (actuator.containsKey("control")) {
                    JsonObject ctrl = actuator["control"].as<JsonObject>();
                    ac.heater_pwm_freq = ctrl["pwm_freq_hz"] | ac.heater_pwm_freq;
                }
            } else if (role == "fan") {
                if (actuator.containsKey("pins")) {
                    JsonObject pins = actuator["pins"].as<JsonObject>();
                    ac.fan_pin = pins["pwm"] | ac.fan_pin;
                }
                if (actuator.containsKey("control")) {
                    JsonObject ctrl = actuator["control"].as<JsonObject>();
                    ac.fan_pwm_freq = ctrl["pwm_freq_hz"] | ac.fan_pwm_freq;
                    ac.cooldown_duration_sec = ctrl["cooldown_sec"] | ac.cooldown_duration_sec;
                }
            }
        }
        config_mgr_->setActuatorConfig(ac);
        
        // Save the full actuators array as object config
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
        
        // Save the full display config as object config
        JsonDocument disp_doc;
        disp_doc.set(display);
        JsonObject disp_obj = disp_doc.as<JsonObject>();
        config_mgr_->setObjectConfig("display", disp_obj);
    }
    
    // Control algorithm
    if (payload.containsKey("control")) {
        JsonObject control = payload["control"].as<JsonObject>();
        
        // Save as generic object config for full persistence
        JsonDocument ctrl_doc;
        ctrl_doc.set(control);
        JsonObject ctrl_obj = ctrl_doc.as<JsonObject>();
        config_mgr_->setObjectConfig("control", ctrl_obj);
        
        // Update PID config if algorithm is pid
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
    
    // Phase 4: Build success response
    JsonObject response = ws_->makeJsonObject();
    response["status"] = "saved";
    
    // Include validation summary if available
    if (hw_parser_) {
        JsonObject validation_obj = response.createNestedObject("validation");
        validation_obj["valid"] = true;
    }
    
    sendResponse(client, "config/hardware/response", response);
    
    if (log_mgr_) {
        // Notify via system log
        Serial.println("[WebSocketServer] Hardware config saved via WebSocket");
    }
}

void WebSocketServer::handleConfigDisplay(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!config_mgr_) {
        sendError(client, "config/display", "Config manager not available");
        return;
    }
    
    ConfigManager::DisplayConfig dc = config_mgr_->getDisplayConfig();
    dc.enabled = payload["enabled"] | dc.enabled;
    dc.driver = payload["driver"] | dc.driver;
    dc.bus_type = payload["bus_type"] | dc.bus_type;
    dc.width = payload["width"] | dc.width;
    dc.height = payload["height"] | dc.height;
    dc.rotation = payload["rotation"] | dc.rotation;
    dc.spi_mosi = payload["spi_mosi"] | dc.spi_mosi;
    dc.spi_sclk = payload["spi_sclk"] | dc.spi_sclk;
    dc.spi_cs = payload["spi_cs"] | dc.spi_cs;
    dc.dc_pin = payload["dc_pin"] | dc.dc_pin;
    dc.rst_pin = payload["rst_pin"] | dc.rst_pin;
    dc.backlight_pin = payload["backlight_pin"] | dc.backlight_pin;
    
    if (payload.containsKey("fields")) {
        JsonArray arr = payload["fields"];
        dc.fields.clear();
        for (JsonVariant v : arr) {
            dc.fields.push_back(v.as<String>());
        }
    }
    
    config_mgr_->setDisplayConfig(dc);
    
    JsonObject response = ws_->makeJsonObject();
    response["status"] = "saved";
    sendResponse(client, "config/display/response", response);
}

void WebSocketServer::handleConfigProfiles(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!config_mgr_) {
        sendError(client, "config/profiles", "Config manager not available");
        return;
    }
    
    String action = payload["action"] | "list";
    
    if (action == "list") {
        auto profiles = config_mgr_->getProfiles();
        JsonObject response = ws_->makeJsonObject();
        JsonArray arr = response.createNestedArray("profiles");
        for (const auto& p : profiles) {
            JsonObject obj = arr.createNestedObject();
            obj["id"] = p.id;
            obj["name_pt"] = p.name_pt;
            obj["name_en"] = p.name_en;
            obj["target_temp_c"] = p.target_temp_c;
            obj["default_duration_min"] = p.default_duration_min;
            obj["target_humidity_pct"] = p.target_humidity_pct;
            obj["is_builtin"] = p.is_builtin;
        }
        sendResponse(client, "config/profiles/list/response", response);
    }
    else if (action == "get") {
        String pid = payload["profile_id"] | "";
        auto p = config_mgr_->getProfile(pid);
        if (!p.id.isEmpty()) {
            JsonObject response = ws_->makeJsonObject();
            JsonObject obj = response.createNestedObject("profile");
            obj["id"] = p.id;
            obj["name_pt"] = p.name_pt;
            obj["name_en"] = p.name_en;
            obj["target_temp_c"] = p.target_temp_c;
            obj["default_duration_min"] = p.default_duration_min;
            obj["target_humidity_pct"] = p.target_humidity_pct;
            obj["is_builtin"] = p.is_builtin;
            sendResponse(client, "config/profiles/get/response", response);
        } else {
            sendError(client, "config/profiles/get", "Profile not found");
        }
    }
    else if (action == "create") {
        ConfigManager::FilamentProfile p;
        p.id = "custom-" + String(millis());
        p.name_pt = payload["name_pt"] | "";
        p.name_en = payload["name_en"] | "";
        p.target_temp_c = payload["target_temp_c"] | 50.0f;
        p.default_duration_min = payload["default_duration_min"] | 240;
        p.target_humidity_pct = payload["target_humidity_pct"] | 15.0f;
        p.is_builtin = false;
        p.created_at = millis();
        p.updated_at = millis();
        
        if (config_mgr_->addProfile(p)) {
            JsonObject response = ws_->makeJsonObject();
            response["status"] = "created";
            response["profile_id"] = p.id;
            sendResponse(client, "config/profiles/create/response", response);
        } else {
            sendError(client, "config/profiles/create", "Failed to create profile");
        }
    }
    else if (action == "update") {
        String pid = payload["profile_id"] | "";
        ConfigManager::FilamentProfile p = config_mgr_->getProfile(pid);
        if (p.id.isEmpty() || p.is_builtin) {
            sendError(client, "config/profiles/update", "Profile not found or read-only");
            return;
        }
        
        p.name_pt = payload["name_pt"] | p.name_pt;
        p.name_en = payload["name_en"] | p.name_en;
        p.target_temp_c = payload["target_temp_c"] | p.target_temp_c;
        p.default_duration_min = payload["default_duration_min"] | p.default_duration_min;
        p.target_humidity_pct = payload["target_humidity_pct"] | p.target_humidity_pct;
        p.updated_at = millis();
        
        if (config_mgr_->updateProfile(p)) {
            JsonObject response = ws_->makeJsonObject();
            response["status"] = "updated";
            sendResponse(client, "config/profiles/update/response", response);
        } else {
            sendError(client, "config/profiles/update", "Failed to update profile");
        }
    }
    else if (action == "delete") {
        String pid = payload["profile_id"] | "";
        if (config_mgr_->deleteProfile(pid)) {
            JsonObject response = ws_->makeJsonObject();
            response["status"] = "deleted";
            sendResponse(client, "config/profiles/delete/response", response);
        } else {
            sendError(client, "config/profiles/delete", "Failed to delete profile");
        }
    }
    else if (action == "reset_defaults") {
        config_mgr_->resetProfilesToDefaults();
        JsonObject response = ws_->makeJsonObject();
        response["status"] = "reset";
        sendResponse(client, "config/profiles/reset/response", response);
    }
}

void WebSocketServer::handlePidCalibrate(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!pid_autotune_) {
        sendError(client, "control/pid_calibrate", "PID auto-tune not available");
        return;
    }
    
    float target = payload["target_temp_c"] | 50.0f;
    int cycles = payload["cycles"] | 5;
    
    // Start async PID auto-tune
    if (pid_autotune_->startCalibration(target, cycles)) {
        JsonObject response = ws_->makeJsonObject();
        response["status"] = "started";
        response["target_temp_c"] = target;
        response["cycles"] = cycles;
        sendResponse(client, "control/pid_calibrate/response", response);
    } else {
        sendError(client, "control/pid_calibrate", "Failed to start calibration");
    }
}

void WebSocketServer::handleStatusSubscribe(AsyncWebSocketClient* client, const JsonObject& payload) {
    for (auto& c : clients_) {
        if (c.id == client->id()) {
            c.subscribed = true;
            break;
        }
    }
}

void WebSocketServer::handleStatusUnsubscribe(AsyncWebSocketClient* client, const JsonObject& payload) {
    for (auto& c : clients_) {
        if (c.id == client->id()) {
            c.subscribed = false;
            break;
        }
    }
}

void WebSocketServer::handleLogsSubscribe(AsyncWebSocketClient* client, const JsonObject& payload) {
    for (auto& c : clients_) {
        if (c.id == client->id()) {
            c.log_subscribed = true;
            break;
        }
    }
}

void WebSocketServer::handleLogsUnsubscribe(AsyncWebSocketClient* client, const JsonObject& payload) {
    for (auto& c : clients_) {
        if (c.id == client->id()) {
            c.log_subscribed = false;
            break;
        }
    }
}

void WebSocketServer::broadcastTelemetry(const JsonObject& telemetry) {
    if (!ws_) return;
    
    // T019c: Enrich the telemetry payload with generic status fields
    // (sensors, actuators, display, control sections)
    buildStatusPayload(const_cast<JsonObject&>(telemetry));
    
    String json;
    serializeJson(telemetry, json);
    
    for (auto& c : clients_) {
        if (c.subscribed) {
            ws_->textAll(json.c_str(), json.length());
        }
    }
}

void WebSocketServer::broadcastFault(FaultCode fault, const String& message) {
    if (!ws_) return;
    
    JsonObject fault_obj = ws_->makeJsonObject();
    fault_obj["fault_code"] = static_cast<int>(fault);
    fault_obj["message"] = message;
    fault_obj["action_taken"] = "Heater MOSFET PWM cut off to 0%. Platform online.";
    fault_obj["timestamp_sec"] = millis() / 1000;
    
    String json;
    serializeJson(fault_obj, json);
    ws_->textAll(json.c_str(), json.length());
}

void WebSocketServer::broadcastLog(const String& line, bool is_drying_log) {
    if (!ws_) return;
    
    JsonObject log_obj = ws_->makeJsonObject();
    log_obj["target_log"] = is_drying_log ? "drying" : "system";
    log_obj["line"] = line;
    
    String json;
    serializeJson(log_obj, json);
    
    for (auto& c : clients_) {
        if (c.log_subscribed) {
            ws_->text(c.id, json.c_str(), json.length());
        }
    }
}

void WebSocketServer::broadcastPidCalibrate(const JsonObject& progress) {
    if (!ws_) return;
    
    String json;
    serializeJson(progress, json);
    ws_->textAll(json.c_str(), json.length());
}

void WebSocketServer::sendResponse(AsyncWebSocketClient* client, const String& topic, const JsonObject& payload) {
    if (!client->canSend()) return;
    
    StaticJsonDocument<1024> doc;
    doc["topic"] = topic;
    doc["payload"] = payload;
    
    String json;
    serializeJson(doc, json);
    client->text(json.c_str(), json.length());
}

void WebSocketServer::sendError(AsyncWebSocketClient* client, const String& topic, const String& error) {
    StaticJsonDocument<512> doc;
    doc["topic"] = topic + "/error";
    doc["payload"]["error"] = error;
    
    String json;
    serializeJson(doc, json);
    client->text(json.c_str(), json.length());
}

// T019c: Generic status payload builder reflecting all configured sensors/actuators
void WebSocketServer::buildStatusPayload(JsonObject& payload) {
    if (!payload.isNull()) {
        // System-level status
        if (state_machine_) {
            payload["status"] = state_machine_->getStateName();
            
            const DryingSession& session = state_machine_->getCurrentSession();
            payload["session_id"] = session.session_id;
            payload["target_temp_c"] = session.target_temp_c;
            payload["target_humidity_pct"] = session.target_humidity_pct;
            payload["elapsed_time_sec"] = session.elapsed_sec;
            payload["remaining_time_sec"] = session.remaining_sec;
        }
        
        // System metrics
        payload["uptime_sec"] = millis() / 1000;
        payload["free_heap_bytes"] = ESP.getFreeHeap();
        payload["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
        payload["chip_model"] = ESP.getChipModel();
        
        // Safety engine state
        if (safety_) {
            payload["safety_faulted"] = safety_->isFaulted();
            if (safety_->isFaulted()) {
                payload["safety_fault_code"] = static_cast<int>(safety_->getLastFault());
                payload["safety_fault_message"] = safety_->getLastFaultMessage();
            }
        }
        
        // Build a "sensors" object reflecting all configured sensors
        if (config_mgr_) {
            JsonObject sensors_obj = payload.createNestedObject("sensors");
            
            // Iterate through the saved sensors array
            JsonObject saved_sensors = config_mgr_->getObjectConfig("sensors");
            if (!saved_sensors.isNull() && saved_sensors.is<JsonArray>()) {
                JsonArray sensor_arr = saved_sensors.as<JsonArray>();
                int idx = 0;
                for (JsonVariant v : sensor_arr) {
                    JsonObject sensor = v.as<JsonObject>();
                    String id = sensor["id"] | ("sensor_" + String(idx));
                    String type = sensor["type"] | "unknown";
                    
                    JsonObject entry = sensors_obj.createNestedObject(id);
                    entry["type"] = type;
                    
                    // Last reading fields are populated by the control loop
                    entry["temperature_c"] = nullptr;
                    entry["humidity_pct"] = nullptr;
                    entry["pressure_hpa"] = nullptr;
                    entry["valid"] = false;
                    
                    // Capabilities
                    if (sensor.containsKey("capabilities")) {
                        JsonArray caps = sensor["capabilities"].as<JsonArray>();
                        JsonArray caps_out = entry.createNestedArray("capabilities");
                        for (JsonVariant c : caps) {
                            caps_out.add(c.as<String>());
                        }
                    }
                    
                    idx++;
                }
            }
            
            // Build "actuators" object reflecting all configured actuators
            JsonObject actuators_obj = payload.createNestedObject("actuators");
            
            JsonObject saved_actuators = config_mgr_->getObjectConfig("actuators");
            if (!saved_actuators.isNull() && saved_actuators.is<JsonArray>()) {
                JsonArray act_arr = saved_actuators.as<JsonArray>();
                int idx = 0;
                for (JsonVariant v : act_arr) {
                    JsonObject actuator = v.as<JsonObject>();
                    String id = actuator["id"] | ("actuator_" + String(idx));
                    String role = actuator["role"] | "custom";
                    String type = actuator["type"] | "unknown";
                    
                    JsonObject entry = actuators_obj.createNestedObject(id);
                    entry["type"] = type;
                    entry["role"] = role;
                    entry["enabled"] = false;
                    entry["power_pct"] = 0.0f;
                    entry["fault"] = false;
                    
                    idx++;
                }
            }
            
            // Build "display" object reflecting configured display
            JsonObject display_obj = payload.createNestedObject("display");
            DisplayConfig dc = config_mgr_->getDisplayConfig();
            display_obj["enabled"] = dc.enabled;
            display_obj["driver"] = dc.driver;
            display_obj["width"] = dc.width;
            display_obj["height"] = dc.height;
            display_obj["rotation"] = dc.rotation;
            
            // Build "control" object reflecting current control algorithm
            JsonObject control_obj = payload.createNestedObject("control");
            JsonObject saved_control = config_mgr_->getObjectConfig("control");
            if (!saved_control.isNull()) {
                control_obj["algorithm"] = saved_control["algorithm"] | "pid";
                control_obj["auto_tune"] = saved_control["auto_tune"] | false;
                if (saved_control.containsKey("parameters")) {
                    JsonObject params = saved_control["parameters"].as<JsonObject>();
                    JsonObject params_out = control_obj.createNestedObject("parameters");
                    for (JsonPair kv : params) {
                        params_out[kv.key()] = kv.value();
                    }
                }
            } else {
                control_obj["algorithm"] = "pid";
                control_obj["auto_tune"] = false;
            }
        }
    }
}

} // namespace filament_dryer