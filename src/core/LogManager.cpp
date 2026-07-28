/**
 * LogManager - Implementation
 * Dual logging: system.log (boot & runtime) and drying.log (per-session)
 */
#include <Arduino.h>
#include "firmware_version.h"
#include "LogManager.hpp"
#include "StateMachine.hpp"

namespace filament_dryer {

LogManager::LogManager() {}

LogManager::~LogManager() {}

bool LogManager::begin() {
    if (initialized_) return true;
    
    if (!LittleFS.begin()) {
        Serial.println("[LogManager] ERROR: LittleFS mount failed");
        return false;
    }
    
    ensureDirectory();
    
    // Rotate system log on boot
    rotateSystemLog();
    
    // Drying log is recreated at each session start via newDryingSession()
    initialized_ = true;
    
    Serial.println("[LogManager] Ready");
    return true;
}

} // namespace filament_dryer