#include "WebSocketServer.hpp"

#include "../core/ConfigManager.hpp"
#include "../core/LogManager.hpp"
#include "../core/PidAutotuneController.hpp"
#include "../core/StateMachine.hpp"
#include "../core/ProfileManager.hpp"
#include "../core/HardwareConfigParser.hpp"
#include "../control/ControlEngine.hpp"

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
    } else if (topic == "config/profiles") {
        handleConfigProfiles(client, payload);
    } else if (topic == "config/control") {
        handleConfigControl(client, payload);
    } else if (topic == "control/pid_calibrate") {
        handlePidCalibrate(client, payload);
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

    if (!state_machine_->isDrying()) {
        sendError(client, "control/stop", "No active drying session");
        return;
    }

    state_machine_->stopDrying(DryingStopReason::USER_STOPPED);
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
    body["status"] = "applied";
    sendResponse(client, "config/hardware", body);
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

void WebSocketServer::handleConfigProfiles(AsyncWebSocketClient* client, const JsonObject& payload) {
    if (!profile_mgr_) {
        sendError(client, "config/profiles", "Profile manager unavailable");
        return;
    }

    const String action = payload["action"] | "list";
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
        }
        sendResponse(client, "config/profiles", body);
        return;
    }

    if (action == "create" || action == "update") {
        FilamentProfile profile;
        profile.id = payload["id"] | "";
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
            sendError(client, "config/profiles", "Failed to save profile");
            return;
        }
        body["status"] = "saved";
        body["id"] = profile.id;
        sendResponse(client, "config/profiles", body);
        return;
    }

    if (action == "delete") {
        const String id = payload["id"] | "";
        if (!profile_mgr_->deleteProfile(id)) {
            sendError(client, "config/profiles", "Failed to delete profile");
            return;
        }
        body["status"] = "deleted";
        body["id"] = id;
        sendResponse(client, "config/profiles", body);
        return;
    }

    if (action == "reset") {
        profile_mgr_->resetToDefaults();
        body["status"] = "reset";
        sendResponse(client, "config/profiles", body);
        return;
    }

    sendError(client, "config/profiles", "Unknown action");
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
    cfg.max_cycles = payload["max_cycles"] | 5;
    cfg.max_temp = payload["max_temp_c"] | 80.0f;

  const bool started = pid_autotune_->startCalibration(
        cfg,
        [](int cycle, int total, float temp, float kp, float ki, float kd, bool done) {
            (void)cycle;
            (void)total;
            (void)temp;
            (void)kp;
            (void)ki;
            (void)kd;
            (void)done;
        },
        [](const PidAutotuneController::Result&) {});

    if (!started) {
        sendError(client, "control/pid_calibrate", "Failed to start calibration");
        return;
    }

    JsonDocument response;
    JsonObject body = response.to<JsonObject>();
    body["status"] = "started";
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
    ws_->textAll(json.c_str());
}

void WebSocketServer::broadcastFault(FaultCode fault, const String& message) {
    if (!ws_) {
        return;
    }

    JsonDocument doc;
    doc["topic"] = "status/fault";
    JsonObject payload = doc["payload"].to<JsonObject>();
    payload["fault"] = static_cast<int>(fault);
    payload["message"] = message;

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
    payload["line"] = line;
    payload["drying"] = is_drying_log;

    String json;
    serializeJson(doc, json);
    ws_->textAll(json.c_str());
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
    payload["status"] = state_machine_->getStateName();
    payload["target_temp_c"] = session.target_temp_c;
    payload["target_humidity_pct"] = session.target_humidity_pct;
    payload["elapsed_time_sec"] = session.elapsed_sec;
    payload["remaining_time_sec"] = session.remaining_sec;
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
    payload["message"] = error;

    String json;
    serializeJson(doc, json);
    client->text(json.c_str());
}

}  // namespace filament_dryer
