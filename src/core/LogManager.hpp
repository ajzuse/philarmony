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
 * LogManager - Dual logging system (system.log and drying.log)
 */
#pragma once

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include "StateMachine.hpp"

namespace filament_dryer {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

enum class LogModule {
    SYSTEM,
    NETWORK,
    SENSOR,
    ACTUATOR,
    DRYING,
    SAFETY,
    DISPLAY_MODULE,
    PID,
    NVS
};

struct LogEntry {
    uint32_t timestamp;
    LogLevel level;
    LogModule module;
    String message;
};

class LogManager {
public:
    static constexpr const char* SYSTEM_LOG_PATH = "/littlefs/system.log";
    static constexpr const char* DRYING_LOG_PATH = "/littlefs/drying.log";
    static constexpr size_t MAX_LOG_LINE_LENGTH = 256;
    static constexpr size_t MAX_SYSTEM_LOG_SIZE = 64 * 1024; // 64KB
    static constexpr size_t MAX_DRYING_LOG_SIZE = 128 * 1024; // 128KB
    
    LogManager();
    ~LogManager();
    
    bool begin();
    
    // System log (persistent across reboots, rotated on boot)
    void logSystem(LogLevel level, LogModule module, const char* format, ...);
    void logSystem(LogLevel level, LogModule module, const String& message);
    
    // Drying log (recreated at each drying cycle start, preserved post-mortem)
    void logDrying(LogLevel level, LogModule module, const char* format, ...);
    void logDrying(LogLevel level, LogModule module, const String& message);
    
    // Structured drying log entry
    void logDryingTelemetry(float temp, float target_temp, float humidity, 
                            float target_humidity, float heater_pct, float fan_pct,
                            uint32_t elapsed_sec, uint32_t remaining_sec);
    
    void logDryingStart(const String& profile_id, float target_temp, 
                        uint16_t max_duration, float target_humidity);
    void logDryingStop(DryingStopReason reason);
    void logDryingFault(const String& fault_code, const String& message);
    
    // Log file management
    void newDryingSession(); // Truncates drying.log at cycle start
    bool rotateSystemLog();  // Rotates system.log if too large
    
    // HTTP download support
    bool getSystemLog(String& output) const;
    bool getDryingLog(String& output) const;
    size_t getSystemLogSize() const;
    size_t getDryingLogSize() const;
    
    // Callback for real-time streaming
    using LogCallback = void(*)(const String& line, bool is_drying_log);
    void setLogCallback(LogCallback cb) { log_cb_ = cb; }

private:
    bool initialized_ = false;
    bool drying_session_active_ = false;
    uint32_t current_session_id_ = 0;
    
    LogCallback log_cb_ = nullptr;
    
    // Format and write
    String formatEntry(LogLevel level, LogModule module, const String& message);
    void writeToFile(const char* path, const String& line, size_t max_size);
    void ensureDirectory();
    
    // Level strings
    static const char* levelToStr(LogLevel level);
    static const char* moduleToStr(LogModule module);
};

} // namespace filament_dryer