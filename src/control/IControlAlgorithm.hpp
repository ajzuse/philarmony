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