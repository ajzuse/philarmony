/**
 * PIDControl - PID temperature control with anti-windup
 */
#pragma once

#include "IControlAlgorithm.hpp"

namespace filament_dryer {

struct PIDConfig {
    float kp = 0;
    float ki = 0;
    float kd = 0;
    float max_integral = 1000;
    float min_output = 0;
    float max_output = 100;
};

class PIDControl : public IControlAlgorithm {
public:
    PIDControl();
    ~PIDControl() override;
    
    bool begin(const JsonObject& config) override;
    float compute(float target, float current, float dt) override;
    void reset() override;
    String getType() const override { return "pid"; }
    String getName() const override { return "PID Control"; }
    bool needsTuning() const override { return true; }
    JsonObject getParameters() override;
    void setParameters(const JsonObject& params) override;
    bool isInitialized() const override { return initialized_; }

private:
    PIDConfig config_;
    float integral_ = 0.0f;
    float last_error_ = 0.0f;
    float last_output_ = 0.0f;
    bool initialized_ = false;
    bool tuning_mode_ = false;
    
    float clampOutput(float output);
};

} // namespace filament_dryer