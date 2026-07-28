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
 * ControlEngine - Manages multiple control algorithms and switches between them
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <functional>
#include "IControlAlgorithm.hpp"

namespace filament_dryer {

class ControlEngine {
public:
    ControlEngine();
    ~ControlEngine();
    
    bool begin();
    
    // Set active algorithm
    bool setAlgorithm(const String& type, const JsonObject& config);
    
    // Get current algorithm
    IControlAlgorithm* getCurrentAlgorithm() const;
    String getCurrentAlgorithmType() const;
    
    // Compute control output using current algorithm
    float compute(float target_temp, float current_temp, float dt);
    
    // List available algorithms
    std::vector<String> listAlgorithms() const;
    
    // Register custom algorithm
    void registerAlgorithm(const String& type, 
                           std::function<IControlAlgorithm*(const JsonObject&)> factory);

private:
    IControlAlgorithm* current_algorithm_ = nullptr;
    String current_type_;
    bool initialized_ = false;
    
    // Factory map for custom algorithms
    std::map<String, std::function<IControlAlgorithm*(const JsonObject&)>> factories_;
};

} // namespace filament_dryer