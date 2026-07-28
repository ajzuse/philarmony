#include "WebSocketServer.hpp"

#include "../core/ConfigManager.hpp"
#include "../core/LogManager.hpp"
#include "../core/PidAutotuneController.hpp"
#include "../core/StateMachine.hpp"

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
                            DriverRegistry* driver_registry) {
    config_mgr_ = config_mgr;
    state_machine_ = state_machine;
    safety_ = safety;
    log_mgr_ = log_mgr;
    pid_autotune_ = pid_autotune;
    hw_parser_ = hw_parser;
    driver_registry_ = driver_registry;

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
    }
}

void WebSocketServer::handleControlStart(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)client;
    (void)payload;
}

void WebSocketServer::handleControlStop(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)client;
    (void)payload;
}

void WebSocketServer::handleConfigHardware(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)client;
    (void)payload;
}

void WebSocketServer::handleConfigDisplay(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)client;
    (void)payload;
}

void WebSocketServer::handleConfigProfiles(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)client;
    (void)payload;
}

void WebSocketServer::handlePidCalibrate(AsyncWebSocketClient* client, const JsonObject& payload) {
    (void)client;
    (void)payload;
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
