/**
 * PluginManager - Implementation
 */
#include "IPlugin.hpp"
#include "../core/ConfigManager.hpp"

namespace filament_dryer {

PluginManager::PluginManager() {}

PluginManager::~PluginManager() {
    for (auto* plugin : plugins_) {
        delete plugin;
    }
    plugins_.clear();
}

bool PluginManager::begin() {
    initialized_ = true;
    return true;
}

bool PluginManager::registerPlugin(IPlugin* plugin) {
    if (!plugin) return false;

    for (auto* p : plugins_) {
        if (p->getName() == plugin->getName()) {
            return false;
        }
    }

    plugins_.push_back(plugin);
    return true;
}

bool PluginManager::unregisterPlugin(const String& name) {
    for (auto it = plugins_.begin(); it != plugins_.end(); ++it) {
        if ((*it)->getName() == name) {
            delete *it;
            plugins_.erase(it);
            return true;
        }
    }
    return false;
}

IPlugin* PluginManager::getPlugin(const String& name) {
    for (auto* p : plugins_) {
        if (p->getName() == name) return p;
    }
    return nullptr;
}

std::vector<String> PluginManager::listPlugins() const {
    std::vector<String> names;
    for (auto* p : plugins_) {
        names.push_back(p->getName());
    }
    return names;
}

void PluginManager::callOnInit(ConfigManager& config) {
    for (auto* p : plugins_) {
        p->onInit(config);
    }
}

void PluginManager::callOnStart() {
    for (auto* p : plugins_) {
        p->onStart();
    }
}

void PluginManager::callOnStop() {
    for (auto* p : plugins_) {
        p->onStop();
    }
}

void PluginManager::callOnShutdown() {
    for (auto* p : plugins_) {
        p->onShutdown();
    }
}

void PluginManager::callOnSessionStart(const DryingSession& session) {
    for (auto* p : plugins_) {
        p->onSessionStart(session);
    }
}

void PluginManager::callOnSessionStop(const DryingSession& session, DryingStopReason reason) {
    for (auto* p : plugins_) {
        p->onSessionStop(session, reason);
    }
}

void PluginManager::callOnTelemetryTick(const StatusPayload& telemetry) {
    for (auto* p : plugins_) {
        p->onTelemetryTick(telemetry);
    }
}

void PluginManager::callOnFault(FaultCode fault, const String& message) {
    for (auto* p : plugins_) {
        p->onFault(fault, message);
    }
}

void PluginManager::callOnConfigChanged(const String& section, const JsonObject& new_config) {
    for (auto* p : plugins_) {
        p->onConfigChanged(section, new_config);
    }
}

bool PluginManager::callWebSocketCommand(const String& topic, const JsonObject& payload,
                                         JsonObject& response) {
    for (auto* p : plugins_) {
        if (p->handleWebSocketCommand(topic, payload, response)) {
            return true;
        }
    }
    return false;
}

bool PluginManager::callHttpRequest(const String& path, const JsonObject& params,
                                    String& response) {
    for (auto* p : plugins_) {
        if (p->handleHttpRequest(path, params, response)) {
            return true;
        }
    }
    return false;
}

}  // namespace filament_dryer
