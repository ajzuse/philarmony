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

/**
 * WebSocketServer - Real-time control and telemetry
 */
#pragma once

#include <Arduino.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include "../core/SafetyEngine.hpp"
#include "../core/PidAutotuneController.hpp"

namespace filament_dryer {

class ConfigManager;
class StateMachine;
class SafetyEngine;
class LogManager;
class HardwareConfigParser;
class DriverRegistry;
class ProfileManager;
class ControlEngine;

class WebSocketServer {
public:
    using HardwareReloadCallback = void(*)();
    using ActuatorCutoffCallback = void(*)();

    WebSocketServer(uint16_t port = 80, const char* path = "/ws");
    ~WebSocketServer();
    
    bool begin(ConfigManager* config_mgr, StateMachine* state_machine,
               SafetyEngine* safety, LogManager* log_mgr,
               PidAutotuneController* pid_autotune,
               HardwareConfigParser* hw_parser = nullptr,
               DriverRegistry* driver_registry = nullptr,
               ProfileManager* profile_mgr = nullptr,
               ControlEngine* control_engine = nullptr);

    void setHardwareReloadCallback(HardwareReloadCallback cb) { hardware_reload_cb_ = cb; }
    void setActuatorCutoffCallback(ActuatorCutoffCallback cb) { actuator_cutoff_cb_ = cb; }
    void setPidCalibrateCallbacks(PidAutotuneController::ProgressCallback progress,
                                  PidAutotuneController::CompleteCallback complete) {
        pid_progress_cb_ = progress;
        pid_complete_cb_ = complete;
    }
    
    void loop();
    
    // Broadcast helpers
    void broadcastTelemetry(const JsonObject& telemetry);
    void broadcastFault(FaultCode fault, const String& message);
    void broadcastLog(const String& line, bool is_drying_log);
    void broadcastPidCalibrate(const JsonObject& progress);
    
    // Generic status payload builder (T019c) - reflects all configured sensors/actuators
    void buildStatusPayload(JsonObject& payload);
    
    // Direct access to AsyncWebSocket for external builders (display task, etc.)
    AsyncWebSocket* getWebSocket() { return ws_; }
    
private:
    uint16_t port_;
    const char* path_;
    AsyncWebSocket* ws_ = nullptr;
    
    ConfigManager* config_mgr_ = nullptr;
    StateMachine* state_machine_ = nullptr;
    SafetyEngine* safety_ = nullptr;
    LogManager* log_mgr_ = nullptr;
    PidAutotuneController* pid_autotune_ = nullptr;
    HardwareConfigParser* hw_parser_ = nullptr;
    DriverRegistry* driver_registry_ = nullptr;
    ProfileManager* profile_mgr_ = nullptr;
    ControlEngine* control_engine_ = nullptr;
    HardwareReloadCallback hardware_reload_cb_ = nullptr;
    ActuatorCutoffCallback actuator_cutoff_cb_ = nullptr;
    PidAutotuneController::ProgressCallback pid_progress_cb_ = nullptr;
    PidAutotuneController::CompleteCallback pid_complete_cb_ = nullptr;
    
    // Client management
    struct ClientInfo {
        uint32_t id;
        bool subscribed = false;
        bool log_subscribed = false;
    };
    std::vector<ClientInfo> clients_;
    
    // Message handlers
    void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                 AwsEventType type, void* arg, uint8_t* data, size_t len);
    void handleMessage(AsyncWebSocketClient* client, uint8_t* data, size_t len);
    void dispatchTopic(AsyncWebSocketClient* client, const JsonObject& doc);
    
    // Command handlers
    void handleControlStart(AsyncWebSocketClient* client, const JsonObject& payload);
    void handleControlStop(AsyncWebSocketClient* client, const JsonObject& payload);
    void handleConfigHardware(AsyncWebSocketClient* client, const JsonObject& payload);
    void handleConfigDisplay(AsyncWebSocketClient* client, const JsonObject& payload);
    void handleConfigProfiles(AsyncWebSocketClient* client, const JsonObject& payload,
                              const String& topic = "config/profiles");
    void handleConfigControl(AsyncWebSocketClient* client, const JsonObject& payload);
    void handlePidCalibrate(AsyncWebSocketClient* client, const JsonObject& payload);
    void handleStatusSubscribe(AsyncWebSocketClient* client, const JsonObject& payload);
    void handleStatusUnsubscribe(AsyncWebSocketClient* client, const JsonObject& payload);
    void handleLogsSubscribe(AsyncWebSocketClient* client, const JsonObject& payload);
    void handleLogsUnsubscribe(AsyncWebSocketClient* client, const JsonObject& payload);
    
    // Response helpers
    void sendResponse(AsyncWebSocketClient* client, const String& topic, const JsonObject& payload);
    void sendError(AsyncWebSocketClient* client, const String& topic, const String& error);
};

} // namespace filament_dryer