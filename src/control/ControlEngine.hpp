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