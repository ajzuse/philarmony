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
    
    logSystem(LogLevel::INFO, LogModule::SYSTEM, 
              "LogManager initialized. Firmware: %s", FIRMWARE_VERSION);
    
    return true;
}

void LogManager::ensureDirectory() {
    // LittleFS doesn't need explicit directory creation for files
}

void LogManager::logSystem(LogLevel level, LogModule module, const char* format, ...) {
    if (!initialized_) return;
    
    char buffer[MAX_LOG_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    String entry = formatEntry(level, module, String(buffer));
    writeToFile(SYSTEM_LOG_PATH, entry, MAX_SYSTEM_LOG_SIZE);
    
    if (log_cb_) log_cb_(entry, false);
}

void LogManager::logSystem(LogLevel level, LogModule module, const String& message) {
    logSystem(level, module, message.c_str());
}

void LogManager::logDrying(LogLevel level, LogModule module, const char* format, ...) {
    if (!initialized_ || !drying_session_active_) return;
    
    char buffer[MAX_LOG_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    String entry = formatEntry(level, module, String(buffer));
    writeToFile(DRYING_LOG_PATH, entry, MAX_DRYING_LOG_SIZE);
    
    if (log_cb_) log_cb_(entry, true);
}

void LogManager::logDrying(LogLevel level, LogModule module, const String& message) {
    logDrying(level, module, message.c_str());
}

void LogManager::logDryingTelemetry(float temp, float target_temp, float humidity,
                                    float target_humidity, float heater_pct, float fan_pct,
                                    uint32_t elapsed_sec, uint32_t remaining_sec) {
    if (!initialized_ || !drying_session_active_) return;
    
    char buffer[MAX_LOG_LINE_LENGTH];
    snprintf(buffer, sizeof(buffer),
             "[DATA][TELEMETRY] Temp: %.1fC/%.1fC, Hum: %.1f/%.1f%%, HeaterPWM: %.1f%%, FanPWM: %.1f%%, Elapsed: %lus, Remaining: %lus",
             temp, target_temp, humidity, target_humidity, 
             heater_pct, fan_pct, (unsigned long)elapsed_sec, (unsigned long)remaining_sec);
    
    String entry = formatEntry(LogLevel::INFO, LogModule::DRYING, String(buffer));
    writeToFile(DRYING_LOG_PATH, entry, MAX_DRYING_LOG_SIZE);
    
    if (log_cb_) log_cb_(entry, true);
}

void LogManager::logDryingStart(const String& profile_id, float target_temp,
                                uint16_t max_duration, float target_humidity) {
    if (!initialized_) return;
    
    drying_session_active_ = true;
    current_session_id_ = millis(); // Use timestamp as session ID
    
    char buffer[MAX_LOG_LINE_LENGTH];
    snprintf(buffer, sizeof(buffer),
             "[INFO][DRYING] Session #%lu started. Profile: %s, Target: %.1fC, Duration: %lum, TargetHum: %.1f%%",
             (unsigned long)current_session_id_, profile_id.c_str(), target_temp, (unsigned long)max_duration, target_humidity);
    
    String entry = formatEntry(LogLevel::INFO, LogModule::DRYING, String(buffer));
    writeToFile(DRYING_LOG_PATH, entry, MAX_DRYING_LOG_SIZE);
    
    if (log_cb_) log_cb_(entry, true);
}

void LogManager::logDryingStop(DryingStopReason reason) {
    if (!initialized_ || !drying_session_active_) return;
    
    const char* reason_str = "unknown";
    switch (reason) {
        case DryingStopReason::COMPLETED: reason_str = "completed"; break;
        case DryingStopReason::USER_STOPPED: reason_str = "user_stopped"; break;
        case DryingStopReason::HUMIDITY_REACHED: reason_str = "humidity_reached"; break;
        case DryingStopReason::MAX_TIME: reason_str = "max_time"; break;
        case DryingStopReason::OVER_TEMP: reason_str = "over_temp"; break;
        case DryingStopReason::SENSOR_ERROR: reason_str = "sensor_error"; break;
        case DryingStopReason::THERMAL_RUNAWAY: reason_str = "thermal_runaway"; break;
    }
    
    char buffer[MAX_LOG_LINE_LENGTH];
    snprintf(buffer, sizeof(buffer),
             "[INFO][DRYING] Session #%lu stopped. Reason: %s",
             (unsigned long)current_session_id_, reason_str);
    
    String entry = formatEntry(LogLevel::INFO, LogModule::DRYING, String(buffer));
    writeToFile(DRYING_LOG_PATH, entry, MAX_DRYING_LOG_SIZE);
    
    if (log_cb_) log_cb_(entry, true);
    
    drying_session_active_ = false;
}

void LogManager::logDryingFault(const String& fault_code, const String& message) {
    if (!initialized_) return;
    
    char buffer[MAX_LOG_LINE_LENGTH];
    snprintf(buffer, sizeof(buffer),
             "[ERROR][SAFETY] %s: %s", fault_code.c_str(), message.c_str());
    
    // Log to both system and drying log
    String entry = formatEntry(LogLevel::ERROR, LogModule::SAFETY, String(buffer));
    writeToFile(SYSTEM_LOG_PATH, entry, MAX_SYSTEM_LOG_SIZE);
    
    if (drying_session_active_) {
        writeToFile(DRYING_LOG_PATH, entry, MAX_DRYING_LOG_SIZE);
        if (log_cb_) log_cb_(entry, true);
    }
    if (log_cb_) log_cb_(entry, false);
}

void LogManager::newDryingSession() {
    if (!initialized_) return;
    
    // Truncate drying log at start of new cycle
    File f = LittleFS.open(DRYING_LOG_PATH, FILE_WRITE);
    if (f) {
        f.close();
    }
    
    drying_session_active_ = false; // Will be set true on next logDryingStart
}

bool LogManager::rotateSystemLog() {
    if (!LittleFS.exists(SYSTEM_LOG_PATH)) return true;
    
    File f = LittleFS.open(SYSTEM_LOG_PATH, FILE_READ);
    if (!f) return false;
    
    size_t size = f.size();
    f.close();
    
    if (size > MAX_SYSTEM_LOG_SIZE) {
        // Truncate to keep last half
        f = LittleFS.open(SYSTEM_LOG_PATH, FILE_READ);
        if (!f) return false;
        
        String content = f.readString();
        f.close();
        
        // Keep last 50% of content
        int keep_from = content.length() / 2;
        int newline = content.indexOf('\n', keep_from);
        if (newline > 0) {
            content = content.substring(newline + 1);
        }
        
        f = LittleFS.open(SYSTEM_LOG_PATH, FILE_WRITE);
        if (f) {
            f.print(content);
            f.close();
            return true;
        }
    }
    return true;
}

bool LogManager::getSystemLog(String& output) const {
    if (!LittleFS.exists(SYSTEM_LOG_PATH)) return false;
    
    File f = LittleFS.open(SYSTEM_LOG_PATH, FILE_READ);
    if (!f) return false;
    
    output = f.readString();
    f.close();
    return true;
}

bool LogManager::getDryingLog(String& output) const {
    if (!LittleFS.exists(DRYING_LOG_PATH)) return false;
    
    File f = LittleFS.open(DRYING_LOG_PATH, FILE_READ);
    if (!f) return false;
    
    output = f.readString();
    f.close();
    return true;
}

size_t LogManager::getSystemLogSize() const {
    if (!LittleFS.exists(SYSTEM_LOG_PATH)) return 0;
    File f = LittleFS.open(SYSTEM_LOG_PATH, FILE_READ);
    if (!f) return 0;
    size_t s = f.size();
    f.close();
    return s;
}

size_t LogManager::getDryingLogSize() const {
    if (!LittleFS.exists(DRYING_LOG_PATH)) return 0;
    File f = LittleFS.open(DRYING_LOG_PATH, FILE_READ);
    if (!f) return 0;
    size_t s = f.size();
    f.close();
    return s;
}

String LogManager::formatEntry(LogLevel level, LogModule module, const String& message) {
    char timestamp[32];
    time_t now = time(nullptr);
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    char buffer[MAX_LOG_LINE_LENGTH];
    snprintf(buffer, sizeof(buffer), "[%s][%s][%s] %s\n",
             timestamp, levelToStr(level), moduleToStr(module), message.c_str());
    
    return String(buffer);
}

void LogManager::writeToFile(const char* path, const String& line, size_t max_size) {
    File f = LittleFS.open(path, FILE_APPEND);
    if (!f) {
        Serial.printf("[LogManager] ERROR: Failed to open %s for append\n", path);
        return;
    }
    
    // Check size before writing
    if (f.size() + line.length() > max_size) {
        f.close();
        // Simple rotation - truncate file
        File fr = LittleFS.open(path, FILE_READ);
        if (fr) {
            String content = fr.readString();
            fr.close();
            
            int keep_from = content.length() / 2;
            int newline = content.indexOf('\n', keep_from);
            if (newline > 0) {
                content = content.substring(newline + 1);
            }
            
            File fw = LittleFS.open(path, FILE_WRITE);
            if (fw) {
                fw.print(content);
                fw.close();
            }
        }
        f = LittleFS.open(path, FILE_APPEND);
        if (!f) return;
    }
    
    f.print(line);
    f.close();
}

const char* LogManager::levelToStr(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
    }
    return "UNKNOWN";
}

const char* LogManager::moduleToStr(LogModule module) {
    switch (module) {
        case LogModule::SYSTEM: return "SYSTEM";
        case LogModule::NETWORK: return "NETWORK";
        case LogModule::SENSOR: return "SENSOR";
        case LogModule::ACTUATOR: return "ACTUATOR";
        case LogModule::DRYING: return "DRYING";
        case LogModule::SAFETY: return "SAFETY";
        case LogModule::DISPLAY_MODULE: return "DISPLAY";
        case LogModule::PID: return "PID";
        case LogModule::NVS: return "NVS";
    }
    return "UNKNOWN";
}

} // namespace filament_dryer