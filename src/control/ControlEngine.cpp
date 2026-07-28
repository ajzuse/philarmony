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
 * ControlEngine - Implementation
 */
#include "ControlEngine.hpp"
#include "PIDControl.hpp"
#include "BangBangControl.hpp"
#include "PWMFeedforwardControl.hpp"
#include <Arduino.h>

namespace filament_dryer {

ControlEngine::ControlEngine() {}

ControlEngine::~ControlEngine() {
    delete current_algorithm_;
}

bool ControlEngine::begin() {
    if (initialized_) return true;
    
    // Register built-in algorithms
    factories_["pid"] = [](const JsonObject& config) -> IControlAlgorithm* {
        return new PIDControl(config);
    };
    factories_["bang_bang"] = [](const JsonObject& config) -> IControlAlgorithm* {
        return new BangBangControl(config);
    };
    factories_["pwm_feedforward"] = [](const JsonObject& config) -> IControlAlgorithm* {
        return new PWMFeedforwardControl(config);
    };
    
    initialized_ = true;
    return true;
}

bool ControlEngine::setAlgorithm(const String& type, const JsonObject& config) {
    if (!initialized_) begin();
    
    auto it = factories_.find(type);
    if (it == factories_.end()) {
        return false;
    }
    
    // Create new algorithm instance
    IControlAlgorithm* new_algo = it->second(config);
    if (!new_algo->begin(config)) {
        delete new_algo;
        return false;
    }
    
    // Clean up old algorithm
    delete current_algorithm_;
    
    current_algorithm_ = new_algo;
    current_type_ = type;
    
    return true;
}

IControlAlgorithm* ControlEngine::getCurrentAlgorithm() const {
    return current_algorithm_;
}

String ControlEngine::getCurrentAlgorithmType() const {
    return current_type_;
}

float ControlEngine::compute(float target_temp, float current_temp, float dt) {
    if (!current_algorithm_ || !current_algorithm_->isInitialized()) {
        return 0.0f;
    }
    
    return current_algorithm_->compute(target_temp, current_temp, dt);
}

std::vector<String> ControlEngine::listAlgorithms() const {
    std::vector<String> result;
    for (const auto& pair : factories_) {
        result.push_back(pair.first);
    }
    return result;
}

void ControlEngine::registerAlgorithm(const String& type, 
                                      std::function<IControlAlgorithm*(const JsonObject&)> factory) {
    factories_[type] = std::move(factory);
}

} // namespace filament_dryer