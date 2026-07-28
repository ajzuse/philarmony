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
 * Plugin System - Extension Interface
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "../core/SafetyEngine.hpp"
#include "../core/StateMachine.hpp"

namespace filament_dryer {

class ConfigManager;

struct StatusPayload {
    String status;
    float chamber_temp_c = NAN;
    float humidity_pct = NAN;
    float heater_power_pct = 0.0f;
};

class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual String getName() const = 0;
    virtual String getVersion() const = 0;
    virtual String getAuthor() const = 0;
    virtual String getDescription() const = 0;

    virtual void onInit(ConfigManager& config) { (void)config; }
    virtual void onStart() {}
    virtual void onStop() {}
    virtual void onShutdown() {}

    virtual void onSessionStart(const DryingSession& session) { (void)session; }
    virtual void onSessionStop(const DryingSession& session, DryingStopReason reason) {
        (void)session;
        (void)reason;
    }
    virtual void onTelemetryTick(const StatusPayload& telemetry) { (void)telemetry; }

    virtual void onFault(FaultCode fault, const String& message) {
        (void)fault;
        (void)message;
    }

    virtual void onConfigChanged(const String& section, const JsonObject& new_config) {
        (void)section;
        (void)new_config;
    }

    virtual bool handleWebSocketCommand(const String& topic, const JsonObject& payload,
                                        JsonObject& response) {
        (void)topic;
        (void)payload;
        (void)response;
        return false;
    }

    virtual bool handleHttpRequest(const String& path, const JsonObject& params,
                                   String& response) {
        (void)path;
        (void)params;
        (void)response;
        return false;
    }
};

class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    bool begin();

    bool registerPlugin(IPlugin* plugin);
    bool unregisterPlugin(const String& name);
    IPlugin* getPlugin(const String& name);
    std::vector<String> listPlugins() const;

    void callOnInit(ConfigManager& config);
    void callOnStart();
    void callOnStop();
    void callOnShutdown();

    void callOnSessionStart(const DryingSession& session);
    void callOnSessionStop(const DryingSession& session, DryingStopReason reason);
    void callOnTelemetryTick(const StatusPayload& telemetry);

    void callOnFault(FaultCode fault, const String& message);
    void callOnConfigChanged(const String& section, const JsonObject& new_config);

    bool callWebSocketCommand(const String& topic, const JsonObject& payload, JsonObject& response);
    bool callHttpRequest(const String& path, const JsonObject& params, String& response);

private:
    std::vector<IPlugin*> plugins_;
    bool initialized_ = false;
};

}  // namespace filament_dryer
