/**
 * ControlEngine - Implementation
 */
#include "ControlEngine.hpp"
#include "PIDControl.hpp"
#include "BangBangControl.hpp"
#include "PWMFeedforwardControl.hpp"

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
    Serial.println("[ControlEngine] Initialized with 3 algorithms");
    return true;
}

bool ControlEngine::setAlgorithm(const String& type, const JsonObject& config) {
    if (!initialized_) begin();
    
    auto it = factories_.find(type);
    if (it == factories_.end()) {
        Serial.printf("[ControlEngine] Unknown algorithm: %s\n", type.c_str());
        return false;
    }
    
    // Create new algorithm instance
    IControlAlgorithm* new_algo = it->second(config);
    if (!new_algo->begin(config)) {
        Serial.printf("[ControlEngine] Failed to initialize algorithm: %s\n", type.c_str());
        delete new_algo;
        return false;
    }
    
    // Clean up old algorithm
    delete current_algorithm_;
    
    current_algorithm_ = new_algo;
    current_type_ = type;
    
    Serial.printf("[ControlEngine] Switched to algorithm: %s\n", type.c_str());
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