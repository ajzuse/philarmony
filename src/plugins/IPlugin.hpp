/**
 * Plugin System - Extension Interface
 */
#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>

namespace filament_dryer {

// Forward declarations
class ConfigManager;
class StateMachine;
class SafetyEngine;
class LogManager;

struct DryingSession;
struct StatusPayload;
struct PidConfig;

class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    // Plugin identification
    virtual String getName() const = 0;
    virtual String getVersion() const = 0;
    virtual String getAuthor() const = 0;
    virtual String getDescription() const = 0;
    
    // Lifecycle hooks
    virtual void onInit(ConfigManager& config) {}
    virtual void onStart() {}
    virtual void onStop() {}
    virtual void onShutdown() {}
    
    // Session hooks
    virtual void onSessionStart(const DryingSession& session) {}
    virtual void onSessionStop(const DryingSession& session, StopReason reason) {}
    virtual void onTelemetryTick(const StatusPayload& telemetry) {}
    
    // Safety hooks
    virtual void onFault(FaultCode fault, const String& message) {}
    
    // Configuration hooks
    virtual void onConfigChanged(const String& section, const JsonObject& new_config) {}
    
    // Custom WebSocket command handlers
    virtual bool handleWebSocketCommand(const String& topic, const JsonObject& payload, JsonObject& response) {
        return false;
    }
    
    // Custom HTTP endpoints
    virtual bool handleHttpRequest(const String& path, const JsonObject& params, String& response) {
        return false;
    }
};

class PluginManager {
public:
    PluginManager();
    ~PluginManager();
    
    bool begin();
    
    // Register a plugin
    bool registerPlugin(IPlugin* plugin);
    
    // Unregister a plugin
    bool unregisterPlugin(const String& name);
    
    // Get plugin by name
    IPlugin* getPlugin(const String& name);
    
    // List all registered plugins
    std::vector<String> listPlugins() const;
    
    // Call lifecycle hooks
    void callOnInit(ConfigManager& config);
    void callOnStart();
    void callOnStop();
    void callOnShutdown();
    
    // Call session hooks
    void callOnSessionStart(const DryingSession& session);
    void callOnSessionStop(const DryingSession& session, StopReason reason);
    void callOnTelemetryTick(const StatusPayload& telemetry);
    
    // Call safety hooks
    void callOnFault(FaultCode fault, const String& message);
    
    // Call config hooks
    void callOnConfigChanged(const String& section, const JsonObject& new_config);
    
    // Call custom command handlers
    bool callWebSocketCommand(const String& topic, const JsonObject& payload, JsonObject& response);
    bool callHttpRequest(const String& path, const JsonObject& params, String& response);

private:
    std::vector<IPlugin*> plugins_;
    bool initialized_ = false;
};

} // namespace filament_dryer