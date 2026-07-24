/**
 * PluginManager - Implementation
 */
#include "IPlugin.hpp"
#include "ConfigManager.hpp"
#include "StateMachine.hpp"
#include "SafetyEngine.hpp"
#include "LogManager.hpp"

namespace filament_dryer {

PluginManager::PluginManager() {}

PluginManager::~PluginManager() {
    for (auto* plugin : plugins_) {
        delete plugin;
    }
    plugins_.clear();
}

bool PluginManager::begin() {
    if (initialized_) return true;
    
    // Call onInit for all plugins
    callOnInit(*ConfigManager::getInstance());
    
    initialized_ = true;
    Serial.printf("[PluginManager] Initialized with %d plugins\n", plugins_.size());
    return true;
}

bool PluginManager::registerPlugin(IPlugin* plugin) {
    if (!plugin) return false;
    
    // Check for duplicate name
    for (auto* p : plugins_) {
        if (p->getName() == plugin->getName()) {
            Serial.printf("[PluginManager] Plugin '%s' already registered\n", plugin->getName().c_str());
            return false;
        }
    }
    
    plugins_.push_back(plugin);
    Serial.printf("[PluginManager] Registered plugin: %s v%s\n", 
                  plugin->getName().c_str(), plugin->getVersion().c_str());
    return true;
}

bool PluginManager::unregisterPlugin(const String& name) {
    for (auto it = plugins_.begin(); it != plugins_.end(); ++it) {
        if ((*it)->getName() == name) {
            delete *it;
            plugins_.erase(it);
            Serial.printf("[PluginManager] Unregistered plugin: %s\n", name.c_str());
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
    for (const auto* p : plugins_) {
        names.push_back(p->getName());
    }
    return names;
}

void PluginManager::callOnInit(ConfigManager& config) {
    for (auto* p : plugins_) {
        try {
            p->onInit(config);
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onInit for plugin: %s\n", p->getName().c_str());
        }
    }
}

void PluginManager::callOnStart() {
    for (auto* p : plugins_) {
        try {
            p->onStart();
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onStart for plugin: %s\n", p->getName().c_str());
        }
    }
}

void PluginManager::callOnStop() {
    for (auto* p : plugins_) {
        try {
            p->onStop();
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onStop for plugin: %s\n", p->getName().c_str());
        }
    }
}

void PluginManager::callOnShutdown() {
    for (auto* p : plugins_) {
        try {
            p->onShutdown();
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onShutdown for plugin: %s\n", p->getName().c_str());
        }
    }
}

void PluginManager::callOnSessionStart(const DryingSession& session) {
    for (auto* p : plugins_) {
        try {
            p->onSessionStart(session);
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onSessionStart for plugin: %s\n", p->getName().c_str());
        }
    }
}

void PluginManager::callOnSessionStop(const DryingSession& session, StopReason reason) {
    for (auto* p : plugins_) {
        try {
            p->onSessionStop(session, reason);
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onSessionStop for plugin: %s\n", p->getName().c_str());
        }
    }
}

void PluginManager::callOnTelemetryTick(const StatusPayload& telemetry) {
    for (auto* p : plugins_) {
        try {
            p->onTelemetryTick(telemetry);
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onTelemetryTick for plugin: %s\n", p->getName().c_str());
        }
    }
}

void PluginManager::callOnFault(FaultCode fault, const String& message) {
    for (auto* p : plugins_) {
        try {
            p->onFault(fault, message);
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onFault for plugin: %s\n", p->getName().c_str());
        }
    }
}

void PluginManager::callOnConfigChanged(const String& section, const JsonObject& new_config) {
    for (auto* p : plugins_) {
        try {
            p->onConfigChanged(section, new_config);
        } catch (...) {
            Serial.printf("[PluginManager] Exception in onConfigChanged for plugin: %s\n", p->getName().c_str());
        }
    }
}

bool PluginManager::callWebSocketCommand(const String& topic, const JsonObject& payload, JsonObject& response) {
    for (auto* p : plugins_) {
        try {
            if (p->handleWebSocketCommand(topic, payload, response)) {
                return true;
            }
        } catch (...) {
            Serial.printf("[PluginManager] Exception in handleWebSocketCommand for plugin: %s\n", p->getName().c_str());
        }
    }
    return false;
}

bool PluginManager::callHttpRequest(const String& path, const JsonObject& params, String& response) {
    for (auto* p : plugins_) {
        try {
            if (p->handleHttpRequest(path, params, response)) {
                return true;
            }
        } catch (...) {
            Serial.printf("[PluginManager] Exception in handleHttpRequest for plugin: %s\n", p->getName().c_str());
        }
    }
    return false;
}

} // namespace filament_dryer