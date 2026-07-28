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

#include "LogManager.hpp"

#include <Arduino.h>
#include <stdarg.h>

namespace filament_dryer {

namespace {

String formatWithArgs(const char* format, va_list args) {
    char buffer[LogManager::MAX_LOG_LINE_LENGTH];
    vsnprintf(buffer, sizeof(buffer), format, args);
    return String(buffer);
}

}  // namespace

LogManager::LogManager() {}

LogManager::~LogManager() {}

bool LogManager::begin() {
    if (initialized_) {
        return true;
    }

    if (!LittleFS.begin()) {
        return false;
    }

    ensureDirectory();
    rotateSystemLog();
    initialized_ = true;
    return true;
}

void LogManager::logSystem(LogLevel level, LogModule module, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const String message = formatWithArgs(format, args);
    va_end(args);
    logSystem(level, module, message);
}

void LogManager::logSystem(LogLevel level, LogModule module, const String& message) {
    writeToFile(SYSTEM_LOG_PATH, formatEntry(level, module, message), MAX_SYSTEM_LOG_SIZE);
}

void LogManager::logDrying(LogLevel level, LogModule module, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const String message = formatWithArgs(format, args);
    va_end(args);
    logDrying(level, module, message);
}

void LogManager::logDrying(LogLevel level, LogModule module, const String& message) {
    writeToFile(DRYING_LOG_PATH, formatEntry(level, module, message), MAX_DRYING_LOG_SIZE);
}

void LogManager::logDryingTelemetry(float temp, float target_temp, float humidity,
                                    float target_humidity, float heater_pct, float fan_pct,
                                    uint32_t elapsed_sec, uint32_t remaining_sec) {
    char buffer[MAX_LOG_LINE_LENGTH];
    snprintf(buffer, sizeof(buffer),
             "telemetry temp=%.2f target=%.2f humidity=%.2f target_humidity=%.2f heater=%.1f fan=%.1f elapsed=%lu remaining=%lu",
             temp, target_temp, humidity, target_humidity, heater_pct, fan_pct,
             static_cast<unsigned long>(elapsed_sec),
             static_cast<unsigned long>(remaining_sec));
    logDrying(LogLevel::INFO, LogModule::DRYING, String(buffer));
}

void LogManager::logDryingStart(const String& profile_id, float target_temp,
                                uint16_t max_duration, float target_humidity) {
    newDryingSession();
    char buffer[MAX_LOG_LINE_LENGTH];
    snprintf(buffer, sizeof(buffer),
             "start profile=%s target=%.2f duration=%u target_humidity=%.2f",
             profile_id.c_str(), target_temp, max_duration, target_humidity);
    logDrying(LogLevel::INFO, LogModule::DRYING, String(buffer));
}

void LogManager::logDryingStop(DryingStopReason reason) {
    logDrying(LogLevel::INFO, LogModule::DRYING,
              String("stop reason=") + static_cast<int>(reason));
}

void LogManager::logDryingFault(const String& fault_code, const String& message) {
    logDrying(LogLevel::ERROR, LogModule::SAFETY,
              String("fault code=") + fault_code + " message=" + message);
}

void LogManager::newDryingSession() {
    ensureDirectory();
    File file = LittleFS.open(DRYING_LOG_PATH, FILE_WRITE);
    if (file) {
        file.close();
        LittleFS.remove(DRYING_LOG_PATH);
    }
    current_session_id_++;
    drying_session_active_ = true;
}

bool LogManager::rotateSystemLog() {
    ensureDirectory();
    const size_t size = getSystemLogSize();
    if (size <= MAX_SYSTEM_LOG_SIZE) {
        return true;
    }
    LittleFS.remove(SYSTEM_LOG_PATH);
    return true;
}

bool LogManager::getSystemLog(String& output) const {
    File file = LittleFS.open(SYSTEM_LOG_PATH, FILE_READ);
    if (!file) {
        output = "";
        return false;
    }
    output = file.readString();
    file.close();
    return true;
}

bool LogManager::getDryingLog(String& output) const {
    File file = LittleFS.open(DRYING_LOG_PATH, FILE_READ);
    if (!file) {
        output = "";
        return false;
    }
    output = file.readString();
    file.close();
    return true;
}

size_t LogManager::getSystemLogSize() const {
    File file = LittleFS.open(SYSTEM_LOG_PATH, FILE_READ);
    if (!file) {
        return 0;
    }
    const size_t size = file.size();
    file.close();
    return size;
}

size_t LogManager::getDryingLogSize() const {
    File file = LittleFS.open(DRYING_LOG_PATH, FILE_READ);
    if (!file) {
        return 0;
    }
    const size_t size = file.size();
    file.close();
    return size;
}

String LogManager::formatEntry(LogLevel level, LogModule module, const String& message) {
    return "[" + String(millis()) + "][" + levelToStr(level) + "][" + moduleToStr(module) + "] " + message + "\n";
}

void LogManager::writeToFile(const char* path, const String& line, size_t max_size) {
    if (!initialized_ && !LittleFS.begin()) {
        return;
    }

    ensureDirectory();

    File file = LittleFS.open(path, FILE_APPEND);
    if (!file) {
        return;
    }
    file.print(line);
    file.close();

    if (log_cb_) {
        log_cb_(line, strcmp(path, DRYING_LOG_PATH) == 0);
    }

    File check = LittleFS.open(path, FILE_READ);
    if (check && check.size() > max_size) {
        check.close();
        LittleFS.remove(path);
    } else if (check) {
        check.close();
    }
}

void LogManager::ensureDirectory() {
    if (!LittleFS.exists("/littlefs")) {
        LittleFS.mkdir("/littlefs");
    }
}

const char* LogManager::levelToStr(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
    }
    return "INFO";
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
    return "SYSTEM";
}

}  // namespace filament_dryer
