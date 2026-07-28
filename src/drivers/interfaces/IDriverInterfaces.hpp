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
 * Abstract Driver Interfaces
 * Base classes for sensor, actuator, and display drivers
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdint.h>

namespace filament_dryer {

// Forward declarations
struct SensorReading;
struct ActuatorState;

// ============================================================
// ISensorDriver - Temperature and Humidity Sensor Interface
// ============================================================
struct SensorReading {
    bool valid = false;
    float temperature = NAN;      // Celsius
    float humidity = NAN;         // Percent RH (0-100)
    float pressure = NAN;         // hPa (optional)
    uint32_t timestamp = 0;       // milliseconds since boot
    String error_message;         // If !valid
};

class ISensorDriver {
public:
    virtual ~ISensorDriver() = default;
    
    // Initialize sensor with configuration
    virtual bool begin(const JsonObject& config) = 0;
    
    // Read current sensor values
    virtual SensorReading read() = 0;
    
    // Get sensor type identifier
    virtual String getType() const = 0;
    
    // Get sensor name for logging
    virtual String getName() const = 0;
    
    // Check if sensor is connected and responding
    virtual bool isConnected() = 0;
    
    // Get last error
    virtual String getLastError() const { return last_error_; }
    
protected:
    String last_error_;
    
    void setError(const String& error) { last_error_ = error; }
    void clearError() { last_error_ = ""; }
};

// ============================================================
// IActuatorDriver - Heater and Fan Control Interface
// ============================================================
struct ActuatorState {
    bool enabled = false;
    float power_pct = 0.0f;       // 0-100%
    bool fault = false;
    String fault_message;
};

class IActuatorDriver {
public:
    virtual ~IActuatorDriver() = default;
    
    // Initialize actuator with configuration
    virtual bool begin(const JsonObject& config) = 0;
    
    // Set power level (0-100%)
    virtual bool setPower(float power_pct) = 0;
    
    // Emergency stop - immediate hardware cutoff
    virtual void emergencyStop() = 0;
    
    // Get current state
    virtual ActuatorState getState() const = 0;
    
    // Get actuator type identifier
    virtual String getType() const = 0;
    
    // Get actuator name for logging
    virtual String getName() const = 0;
    
    // Check if actuator is responding
    virtual bool isHealthy() const = 0;
    
protected:
    String last_error_;
};

// ============================================================
// IDisplayDriver - Display Rendering Interface
// ============================================================
struct DisplayMetrics {
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t rotation = 0;
    String driver_name;
};

class IDisplayDriver {
public:
    virtual ~IDisplayDriver() = default;
    
    // Initialize display with configuration
    virtual bool begin(const JsonObject& config) = 0;
    
    // Clear display
    virtual void clear() = 0;
    
    // Update display with status fields
    virtual bool update(const JsonObject& status_fields) = 0;
    
    // Display error message
    virtual void showError(const String& message) = 0;
    
    // Display boot/splash screen
    virtual void showBootScreen(const String& firmware_version) = 0;
    
    // Get display metrics
    virtual DisplayMetrics getMetrics() const = 0;
    
    // Get display type identifier
    virtual String getType() const = 0;
    
    // Get display name for logging
    virtual String getName() const = 0;
    
    // Check if display is connected and responding
    virtual bool isConnected() = 0;
    
    // Set brightness (0-255)
    virtual void setBrightness(uint8_t brightness) = 0;
    
    // Sleep/wake for power saving
    virtual void sleep() = 0;
    virtual void wake() = 0;

protected:
    String last_error_;
};

} // namespace filament_dryer