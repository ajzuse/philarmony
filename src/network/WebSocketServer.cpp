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

#include "WebSocketServer.hpp"

#include "../core/ConfigManager.hpp"
#include "../core/LogManager.hpp"
#include "../core/PidAutotuneController.hpp"
#include "../core/StateMachine.hpp"
#include "../core/ProfileManager.hpp"
#include "../core/HardwareConfigParser.hpp"
#include "../control/ControlEngine.hpp"
#include "../plugins/IPlugin.hpp"

#include <algorithm>

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
                            DriverRegistry* driver_registry,
                            ProfileManager* profile_mgr,
                            ControlEngine* control_engine) {
    config_mgr_ = config_mgr;
    state_machine_ = state_machine;
    safety_ = safety;
    log_mgr_ = log_mgr;
    pid_autotune_ = pid_autotune;
    hw_parser_ = hw_parser;
    driver_registry_ = driver_registry;
    profile_mgr_ = profile_mgr;
    control_engine_ = control_engine;

    if (!ws_) {
        ws_ = new AsyncWebSocket(path_);
        ws_->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                            AwsEventType type, void* arg, uint8_t* data, size_t len) {
            onEvent(server, client, type, arg, data, len);
        });
    }

    return true;
}

void WebSocketServer::loop() {
    if (ws_) {
        ws_->cleanupClients();
    }
}

void WebSocketServer::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                              AwsEventType type, void* arg, uint8_t* data, size_t len) {
    (void)server;
    (void)arg;

    if (!client) {
        return;
    }

    switch (type) {
        case WS_EVT_CONNECT: {
            ClientInfo info;
            info.id = client->id();
            clients_.push_back(info);
            break;
        }
        case WS_EVT_DISCONNECT:
            clients_.erase(
                std::remove_if(clients_.begin(), clients_.end(),
                               [client](const ClientInfo& info) { return info.id == client->id(); }),
                clients_.end());
            break;
        case WS_EVT_DATA:
            handleMessage(client, data, len);
            break;
        case WS_EVT_PING:
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void WebSocketServer::handleMessage(AsyncWebSocketClient* client, uint8_t* data, size_t len) {
    if (!client || !data || len == 0) {
        return;
    }

    String body;
    body.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        body += static_cast<char>(data[i]);
    }

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok) {
        return;
    }

    dispatchTopic(client, doc.as<JsonObject>());
}

void WebSocketServer::dispatchTopic(AsyncWebSocketClient* client, const JsonObject& doc) {
    JsonObject payload = doc["payload"].as<JsonObject>();
    const String topic = doc["topic"] | "";

    if (topic == "status/subscribe") {
        handleStatusSubscribe(client, payload);
    } else if (topic == "status/unsubscribe") {
        handleStatusUnsubscribe(client, payload);
    } else if (topic == "logs/subscribe") {
        handleLogsSubscribe(client, payload);
    } else if (topic == "logs/unsubscribe") {
        handleLogsUnsubscribe(client, payload);
    } else if (topic == "control/start") {
        handleControlStart(client, payload);
    } else if (topic == "control/stop") {
        handleControlStop(client, payload);
    } else if (topic == "config/hardware") {
        handleConfigHardware(client, payload);
    } else if (topic == "config/display") {
        handleConfigDisplay(client, payload);
    } else if (topic == "config/profiles" || topic.startsWith("config/profiles/")) {
        handleConfigProfiles(client, payload, topic);
    } else if (topic == "config/control") {
        handleConfigControl(client, payload);
    } else if (topic == "control/pid_calibrate") {
        handlePidCalibrate(client, payload);
    } else if (plugin_mgr_) {
        JsonDocument respDoc;
        JsonObject response = respDoc.to<JsonObject>();
        if (plugin_mgr_->callWebSocketCommand(topic, payload, response)) {
            sendResponse(client, topic + "/response", response);
        } else {
            sendError(client, topic, "Unknown topic");
        }
    } else {
        sendError(client, topic, "Unknown topic");
    }
}

void WebSocketServer::handleControlStart(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!state_machine_ || !profile_mgr_) {
        sendError(client, "control/start", "System not ready");
        return;
    }

    if (!state_machine_->isReady() && state_machine_->getState() != SystemState::STOPPED) {
        sendError(client, "control/start", "Invalid state for start");
        return;
    }

    DryingSession session;
    if (!profile_mgr_->buildSessionFromRequest(payload, session)) {
        sendError(client, "control/start", "Invalid start parameters");
        return;
    }

    if (!state_machine_->startDrying(session)) {
        sendError(client, "control/start", "Failed to start drying session");
        return;
    }

    if (log_mgr_) {
        log_mgr_->logDryingStart(session.profile_id, session.target_temp_c,
                                 session.max_duration_min, session.target_humidity_pct);
    }

    JsonDocument response;
    JsonObject body = response.to<JsonObject>();
    body["status"] = "started";
    body["profile_id"] = session.profile_id;
    sendResponse(client, "control/start", body);
}

void WebSocketServer::handleControlStop(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)payload;
    if (!state_machine_) {
        sendError(client, "control/stop", "System not ready");
        return;
    }

    if (!state_machine_->isDrying() && !state_machine_->isCoolingDown()) {
        sendError(client, "control/stop", "No active drying session");
        return;
    }

    state_machine_->stopDrying(DryingStopReason::USER_STOPPED);
    if (actuator_cutoff_cb_) {
        actuator_cutoff_cb_();
    }
    if (log_mgr_) {
        log_mgr_->logDryingStop(DryingStopReason::USER_STOPPED);
    }

    JsonDocument response;
    JsonObject body = response.to<JsonObject>();
    body["status"] = "stopped";
    sendResponse(client, "control/stop", body);
}

void WebSocketServer::handleConfigHardware(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!config_mgr_ || !hw_parser_) {
        sendError(client, "config/hardware", "Hardware parser unavailable");
        return;
    }

    if (state_machine_ && (state_machine_->isDrying() || state_machine_->isCoolingDown())) {
        sendError(client, "config/hardware",
                  "Cannot reload hardware while drying or cooling down");
        return;
    }

    SensorConfig sensor_cfg;
    ActuatorConfig actuator_cfg;
    DisplayConfig display_cfg;
    ControlConfig control_cfg;
    auto result = hw_parser_->parse(payload, sensor_cfg, actuator_cfg, display_cfg, control_cfg);
    if (!result.valid) {
        String message = result.errors.empty() ? "Validation failed" : result.errors[0];
        sendError(client, "config/hardware", message);
        return;
    }

    config_mgr_->setSensorConfig(sensor_cfg);
    config_mgr_->setActuatorConfig(actuator_cfg);
    config_mgr_->setDisplayConfig(display_cfg);
    config_mgr_->setObjectConfig("control", payload["control"].as<JsonObject>());
    config_mgr_->save();

    if (hardware_reload_cb_) {
        hardware_reload_cb_();
    }

    JsonDocument response;
    JsonObject body = response.to<JsonObject>();
    body["status"] = "saved";
    sendResponse(client, "config/hardware/response", body);
}

void WebSocketServer::handleConfigDisplay(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!config_mgr_) {
        sendError(client, "config/display", "Config manager unavailable");
        return;
    }

    DisplayConfig display_cfg = config_mgr_->getDisplayConfig();
    display_cfg.enabled = payload["enabled"] | display_cfg.enabled;
    display_cfg.driver = payload["driver"] | display_cfg.driver;
    display_cfg.bus_type = payload["bus_type"] | display_cfg.bus_type;
    display_cfg.width = payload["width"] | display_cfg.width;
    display_cfg.height = payload["height"] | display_cfg.height;
    display_cfg.rotation = payload["rotation"] | display_cfg.rotation;
    display_cfg.spi_mosi = payload["spi_mosi"] | display_cfg.spi_mosi;
    display_cfg.spi_sclk = payload["spi_sclk"] | display_cfg.spi_sclk;
    display_cfg.spi_cs = payload["spi_cs"] | display_cfg.spi_cs;
    display_cfg.dc_pin = payload["dc_pin"] | display_cfg.dc_pin;
    display_cfg.rst_pin = payload["rst_pin"] | display_cfg.rst_pin;
    display_cfg.backlight_pin = payload["backlight_pin"] | display_cfg.backlight_pin;
    display_cfg.i2c_sda = payload["i2c_sda"] | payload["sda_pin"] | display_cfg.i2c_sda;
    display_cfg.i2c_scl = payload["i2c_scl"] | payload["scl_pin"] | display_cfg.i2c_scl;
    display_cfg.i2c_address = payload["i2c_address"] | payload["address"] | display_cfg.i2c_address;

    if (payload.containsKey("refresh_rate_hz")) {
        display_cfg.refresh_rate_hz = payload["refresh_rate_hz"] | display_cfg.refresh_rate_hz;
        if (display_cfg.refresh_rate_hz < 1) display_cfg.refresh_rate_hz = 1;
        if (display_cfg.refresh_rate_hz > 5) display_cfg.refresh_rate_hz = 5;
    }

    if (payload.containsKey("fields") && payload["fields"].is<JsonArray>()) {
        display_cfg.fields.clear();
        for (JsonVariant v : payload["fields"].as<JsonArray>()) {
            display_cfg.fields.push_back(v.as<String>());
        }
    }

    if (payload["layout"].is<JsonObject>()) {
        JsonObject layout = payload["layout"].as<JsonObject>();
        if (layout.containsKey("font_scaling")) {
            display_cfg.font_scaling = layout["font_scaling"].as<String>();
        }
        if (layout.containsKey("compact_mode")) {
            display_cfg.compact_mode = layout["compact_mode"] | display_cfg.compact_mode;
        }
        if (layout.containsKey("fields") && layout["fields"].is<JsonArray>()) {
            display_cfg.fields.clear();
            for (JsonVariant v : layout["fields"].as<JsonArray>()) {
                display_cfg.fields.push_back(v.as<String>());
            }
        }
        if (layout.containsKey("refresh_rate_hz")) {
            display_cfg.refresh_rate_hz = layout["refresh_rate_hz"] | display_cfg.refresh_rate_hz;
            if (display_cfg.refresh_rate_hz < 1) display_cfg.refresh_rate_hz = 1;
            if (display_cfg.refresh_rate_hz > 5) display_cfg.refresh_rate_hz = 5;
        }
    }
    if (payload.containsKey("font_scaling")) {
        display_cfg.font_scaling = payload["font_scaling"].as<String>();
    }
    if (payload.containsKey("compact_mode")) {
        display_cfg.compact_mode = payload["compact_mode"] | display_cfg.compact_mode;
    }

    config_mgr_->setDisplayConfig(display_cfg);
    config_mgr_->save();

    if (hardware_reload_cb_) {
        hardware_reload_cb_();
    }

    JsonDocument response;
    JsonObject body = response.to<JsonObject>();
    body["status"] = "saved";
    sendResponse(client, "config/display", body);
}

void WebSocketServer::handleConfigProfiles(AsyncWebSocketClient* client, const JsonObject& payload,
                                           const String& topic) {
    if (!profile_mgr_) {
        sendError(client, topic, "Profile manager unavailable");
        return;
    }

    String action = payload["action"] | "";
    if (topic == "config/profiles/list") action = "list";
    else if (topic == "config/profiles/get") action = "get";
    else if (topic == "config/profiles/create") action = "create";
    else if (topic == "config/profiles/update") action = "update";
    else if (topic == "config/profiles/delete") action = "delete";
    else if (topic == "config/profiles/reset_defaults") action = "reset";
    else if (action.isEmpty()) action = "list";

    const String response_topic = (topic == "config/profiles") ? String("config/profiles")
                                                               : (topic + "/response");

    JsonDocument response;
    JsonObject body = response.to<JsonObject>();

    if (action == "list") {
        JsonArray profiles = body["profiles"].to<JsonArray>();
        for (const auto& profile : profile_mgr_->listProfiles()) {
            JsonObject item = profiles.add<JsonObject>();
            item["id"] = profile.id;
            item["name_pt"] = profile.name_pt;
            item["name_en"] = profile.name_en;
            item["target_temp_c"] = profile.target_temp_c;
            item["default_duration_min"] = profile.default_duration_min;
            item["target_humidity_pct"] = profile.target_humidity_pct;
            item["is_builtin"] = profile.is_builtin;
            item["created_at"] = profile.created_at;
            item["updated_at"] = profile.updated_at;
        }
        sendResponse(client, response_topic, body);
        return;
    }

    if (action == "get") {
        const String id = payload["profile_id"] | payload["id"] | "";
        FilamentProfile profile = profile_mgr_->getProfile(id);
        if (profile.id.isEmpty()) {
            sendError(client, topic, "Profile not found");
            return;
        }
        JsonObject profile_obj = body["profile"].to<JsonObject>();
        profile_obj["id"] = profile.id;
        profile_obj["name_pt"] = profile.name_pt;
        profile_obj["name_en"] = profile.name_en;
        profile_obj["target_temp_c"] = profile.target_temp_c;
        profile_obj["default_duration_min"] = profile.default_duration_min;
        profile_obj["target_humidity_pct"] = profile.target_humidity_pct;
        profile_obj["is_builtin"] = profile.is_builtin;
        profile_obj["created_at"] = profile.created_at;
        profile_obj["updated_at"] = profile.updated_at;
        sendResponse(client, response_topic, body);
        return;
    }

    if (action == "create" || action == "update") {
        FilamentProfile profile;
        profile.id = payload["profile_id"] | payload["id"] | "";
        profile.name_pt = payload["name_pt"] | profile.id;
        profile.name_en = payload["name_en"] | profile.id;
        profile.target_temp_c = payload["target_temp_c"] | 50.0f;
        profile.default_duration_min = payload["default_duration_min"] | 240;
        profile.target_humidity_pct = payload["target_humidity_pct"] | 15.0f;
        profile.is_builtin = false;
        profile.updated_at = millis();

        const bool ok = (action == "create") ? profile_mgr_->createProfile(profile)
                                             : profile_mgr_->updateProfile(profile);
        if (!ok) {
            sendError(client, topic, "Failed to save profile");
            return;
        }
        body["status"] = (action == "create") ? "created" : "updated";
        body["profile_id"] = profile.id;
        sendResponse(client, response_topic, body);
        return;
    }

    if (action == "delete") {
        const String id = payload["profile_id"] | payload["id"] | "";
        if (!profile_mgr_->deleteProfile(id)) {
            sendError(client, topic, "Failed to delete profile");
            return;
        }
        body["status"] = "deleted";
        body["id"] = id;
        sendResponse(client, response_topic, body);
        return;
    }

    if (action == "reset") {
        profile_mgr_->resetToDefaults();
        body["status"] = "reset";
        sendResponse(client, response_topic, body);
        return;
    }

    sendError(client, topic, "Unknown action");
}

void WebSocketServer::handleConfigControl(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!config_mgr_ || !control_engine_) {
        sendError(client, "config/control", "Control engine unavailable");
        return;
    }

    const String algorithm = payload["algorithm"] | "pid";
    JsonObject parameters = payload["parameters"].as<JsonObject>();
    if (!control_engine_->setAlgorithm(algorithm, parameters)) {
        sendError(client, "config/control", "Failed to apply control algorithm");
        return;
    }

    config_mgr_->setObjectConfig("control", payload);
    config_mgr_->save();

    if (safety_refresh_cb_) {
        safety_refresh_cb_();
    }

    JsonDocument response;
    JsonObject body = response.to<JsonObject>();
    body["status"] = "applied";
    body["algorithm"] = algorithm;
    sendResponse(client, "config/control", body);
}

void WebSocketServer::handlePidCalibrate(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!pid_autotune_) {
        sendError(client, "control/pid_calibrate", "PID autotune unavailable");
        return;
    }

    if (pid_autotune_->isRunning()) {
        sendError(client, "control/pid_calibrate", "Calibration already running");
        return;
    }

    PidAutotuneController::Config cfg;
    cfg.target_temp = payload["target_temp_c"] | 50.0f;
    cfg.pwm_step = payload["pwm_step"] | 80.0f;
    cfg.max_cycles = payload["cycles"] | payload["max_cycles"] | 5;
    cfg.max_temp = payload["max_temp_c"] | 80.0f;
    cfg.algorithm = payload["algorithm"] | "pid";
    if (cfg.algorithm != "pid" && cfg.algorithm != "bang_bang" &&
        cfg.algorithm != "pwm_feedforward") {
        cfg.algorithm = "pid";
    }

  const bool started = pid_autotune_->startCalibration(
        cfg,
        pid_progress_cb_ ? pid_progress_cb_
                         : [](int, int, float, float, float, float, bool) {},
        pid_complete_cb_ ? pid_complete_cb_
                         : [](const PidAutotuneController::Result&) {});

    if (!started) {
        sendError(client, "control/pid_calibrate", "Failed to start calibration");
        return;
    }

    JsonDocument response;
    JsonObject body = response.to<JsonObject>();
    body["status"] = "started";
    body["algorithm"] = cfg.algorithm;
    sendResponse(client, "control/pid_calibrate", body);
}

void WebSocketServer::handleStatusSubscribe(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)payload;
    for (auto& info : clients_) {
        if (info.id == client->id()) {
            info.subscribed = true;
            return;
        }
    }
}

void WebSocketServer::handleStatusUnsubscribe(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)payload;
    for (auto& info : clients_) {
        if (info.id == client->id()) {
            info.subscribed = false;
            return;
        }
    }
}

void WebSocketServer::handleLogsSubscribe(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)payload;
    for (auto& info : clients_) {
        if (info.id == client->id()) {
            info.log_subscribed = true;
            return;
        }
    }
}

void WebSocketServer::handleLogsUnsubscribe(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)payload;
    for (auto& info : clients_) {
        if (info.id == client->id()) {
            info.log_subscribed = false;
            return;
        }
    }
}

void WebSocketServer::broadcastTelemetry(const JsonObject& telemetry) {
    if (!ws_) {
        return;
    }

    JsonDocument doc;
    doc["topic"] = "status/update";
    doc["payload"] = telemetry;

    String json;
    serializeJson(doc, json);
    for (const auto& info : clients_) {
        if (!info.subscribed) {
            continue;
        }
        AsyncWebSocketClient* client = ws_->client(info.id);
        if (client && client->canSend()) {
            client->text(json.c_str());
        }
    }
}

void WebSocketServer::broadcastFault(FaultCode fault, const String& message) {
    if (!ws_) {
        return;
    }

    const char* fault_code = "NONE";
    switch (fault) {
        case FaultCode::SENSOR_DISCONNECT: fault_code = "SENSOR_DISCONNECT"; break;
        case FaultCode::OVER_TEMPERATURE: fault_code = "OVER_TEMPERATURE"; break;
        case FaultCode::THERMAL_RUNAWAY: fault_code = "THERMAL_RUNAWAY"; break;
        case FaultCode::I2C_BUS_LOCKUP: fault_code = "I2C_BUS_LOCKUP"; break;
        case FaultCode::SPI_BUS_ERROR: fault_code = "SPI_BUS_ERROR"; break;
        case FaultCode::ACTUATOR_FAULT: fault_code = "ACTUATOR_FAULT"; break;
        case FaultCode::SENSOR_RATE_OF_CHANGE: fault_code = "SENSOR_RATE_OF_CHANGE"; break;
        case FaultCode::NVS_CORRUPT: fault_code = "NVS_CORRUPT"; break;
        case FaultCode::WATCHDOG_RESET: fault_code = "WATCHDOG_RESET"; break;
        default: break;
    }

    JsonDocument doc;
    doc["topic"] = "status/fault";
    JsonObject payload = doc["payload"].to<JsonObject>();
    payload["fault_code"] = fault_code;
    payload["fault"] = static_cast<int>(fault);
    payload["message"] = message;
    payload["action_taken"] = "Actuators cut off. Platform online.";
    payload["timestamp_sec"] = millis() / 1000UL;

    String json;
    serializeJson(doc, json);
    ws_->textAll(json.c_str());
}

void WebSocketServer::broadcastLog(const String& line, bool is_drying_log) {
    if (!ws_) {
        return;
    }

    JsonDocument doc;
    doc["topic"] = "logs/stream";
    JsonObject payload = doc["payload"].to<JsonObject>();
    payload["target_log"] = is_drying_log ? "drying" : "system";
    payload["line"] = line;
    payload["drying"] = is_drying_log;

    String json;
    serializeJson(doc, json);
    for (const auto& info : clients_) {
        if (!info.log_subscribed) {
            continue;
        }
        AsyncWebSocketClient* client = ws_->client(info.id);
        if (client && client->canSend()) {
            client->text(json.c_str());
        }
    }
}

void WebSocketServer::broadcastPidCalibrate(const JsonObject& progress) {
    if (!ws_) {
        return;
    }

    JsonDocument doc;
    doc["topic"] = "status/pid_calibrate";
    doc["payload"] = progress;

    String json;
    serializeJson(doc, json);
    ws_->textAll(json.c_str());
}

void WebSocketServer::buildStatusPayload(JsonObject& payload) {
    if (!state_machine_) {
        return;
    }

    const DryingSession& session = state_machine_->getCurrentSession();
    payload["status"] = state_machine_->getStatusStreamName();
    payload["target_temp_c"] = session.target_temp_c;
    payload["target_humidity_pct"] = session.target_humidity_pct;
    payload["elapsed_time_sec"] = session.elapsed_sec;
    payload["remaining_time_sec"] = session.remaining_sec;
    payload["heater_on"] = session.heater_on;
    payload["heater_power_pct"] = session.heater_power_pct;
    payload["exhaust_fan_on"] = session.exhaust_fan_on;
    payload["exhaust_fan_power_pct"] = session.exhaust_fan_power_pct;
    payload["chamber_temp_c"] = session.current_temp_c;
    payload["humidity_pct"] = session.current_humidity_pct;

    if (session.session_id > 0) {
        payload["session_id"] = session.session_id;
        payload["stop_reason"] = StateMachine::stopReasonToString(session.stop_reason);
    }

    if (config_mgr_) {
        const SensorConfig sensor = config_mgr_->getSensorConfig();
        payload["sensor_type"] = sensor.type;
        const ActuatorConfig actuator = config_mgr_->getActuatorConfig();
        payload["heater_type"] = actuator.heater_type;
        payload["fan_type"] = actuator.fan_type;
    }
}

void WebSocketServer::sendResponse(AsyncWebSocketClient* client, const String& topic, const JsonObject& payload) {
    if (!client || !client->canSend()) {
        return;
    }

    JsonDocument doc;
    doc["topic"] = topic;
    doc["payload"] = payload;

    String json;
    serializeJson(doc, json);
    client->text(json.c_str());
}

void WebSocketServer::sendError(AsyncWebSocketClient* client, const String& topic, const String& error) {
    if (!client || !client->canSend()) {
        return;
    }

    JsonDocument doc;
    doc["topic"] = topic + "/error";
    JsonObject payload = doc["payload"].to<JsonObject>();
    payload["error"] = error;
    payload["message"] = error;

    String json;
    serializeJson(doc, json);
    client->text(json.c_str());
}

}  // namespace filament_dryer
