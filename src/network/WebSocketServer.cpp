/**
 * WebSocketServer - Implementation
 */
#include "WebSocketServer.hpp"
#include "ConfigManager.hpp"
#include "StateMachine.hpp"
#include "SafetyEngine.hpp"
#include "LogManager.hpp"

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
                            PidAutotuneController* pid_autotune) {
    config_mgr_ = config_mgr;
    state_machine_ = state_machine;
    safety_ = safety;
    log_mgr_ = log_mgr;
    pid_autotune_ = pid_autotune;
    
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
            
        case WS_EVT_DATA:
            handleMessage(client, *reinterpret_cast<JsonObject*>(arg));
            break;
            
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void WebSocketServer::handleMessage(AsyncWebSocketClient* client, const JsonObject& doc) {
    String topic = doc["topic"] | "";
    JsonObject payload = doc["payload"] | JsonObject();
    
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
    } else if (topic == "config/profiles/create") {
        handleConfigProfiles(client, payload);
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
    
    StateMachine::DryingSession session;
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
    StateMachine::DryingStopReason stop_reason = StateMachine::DryingStopReason::USER_STOPPED;
    
    if (reason == "humidity_reached") stop_reason = StateMachine::DryingStopReason::HUMIDITY_REACHED;
    else if (reason == "max_time") stop_reason = StateMachine::DryingStopReason::MAX_TIME;
    else if (reason == "over_temp") stop_reason = StateMachine::DryingStopReason::OVER_TEMP;
    else if (reason == "sensor_error") stop_reason = StateMachine::DryingStopReason::SENSOR_ERROR;
    
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
    
    // Sensor config
    if (payload.containsKey("sensor")) {
        JsonObject sensor = payload["sensor"];
        ConfigManager::SensorConfig sc = config_mgr_->getSensorConfig();
        sc.type = sensor["type"] | sc.type;
        sc.is_integrated = sensor["is_integrated"] | sc.is_integrated;
        sc.i2c_bus = sensor["i2c_bus"] | sc.i2c_bus;
        sc.i2c_address = sensor["i2c_address"] | sc.i2c_address;
        sc.gpio_pin = sensor["gpio_pin"] | sc.gpio_pin;
        sc.sda_pin = sensor["sda_pin"] | sc.sda_pin;
        sc.scl_pin = sensor["scl_pin"] | sc.scl_pin;
        config_mgr_->setSensorConfig(sc);
    }
    
    // Actuator config
    if (payload.containsKey("actuators")) {
        JsonObject act = payload["actuators"];
        ConfigManager::ActuatorConfig ac = config_mgr_->getActuatorConfig();
        ac.heater_pin = act["heater_pin"] | ac.heater_pin;
        ac.heater_pwm_freq = act["heater_pwm_freq"] | ac.heater_pwm_freq;
        ac.heater_max_power_pct = act["heater_max_power_pct"] | ac.heater_max_power_pct;
        ac.fan_mode = act["fan_mode"] | ac.fan_mode;
        ac.fan_pin = act["fan_pin"] | ac.fan_pin;
        ac.fan_pwm_freq = act["fan_pwm_freq"] | ac.fan_pwm_freq;
        ac.cooldown_duration_sec = act["cooldown_duration_sec"] | ac.cooldown_duration_sec;
        config_mgr_->setActuatorConfig(ac);
    }
    
    JsonObject response = ws_->makeJsonObject();
    response["status"] = "saved";
    sendResponse(client, "config/hardware/response", response);
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

} // namespace filament_dryer