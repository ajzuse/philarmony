/**
 * IControlAlgorithm - Interface for temperature control algorithms
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

namespace filament_dryer {

class IControlAlgorithm {
public:
    virtual ~IControlAlgorithm() = default;
    
    // Initialize with configuration
    virtual bool begin(const JsonObject& config) = 0;
    
    // Compute control output
    // target_temp: desired temperature in °C
    // current_temp: measured temperature in °C
    // dt: time since last call in seconds
    // Returns: control output (0-100% for PWM)
    virtual float compute(float target_temp, float current_temp, float dt) = 0;
    
    // Reset internal state (integral, previous error, etc.)
    virtual void reset() = 0;
    
    // Get algorithm type identifier
    virtual String getType() const = 0;
    
    // Check if algorithm is initialized
    virtual bool isInitialized() const = 0;
    
    // Additional methods for algorithm metadata and configuration
    virtual String getName() const = 0;
    virtual bool needsTuning() const = 0;
    virtual JsonObject getParameters() = 0;
    virtual void setParameters(const JsonObject& params) = 0;
};

} // namespace filament_dryer