/**
 * PWMFeedforwardControl - Feedforward Control with PWM
 * Uses base PWM + temperature coefficient for predictive control
 */
#pragma once

#include "IControlAlgorithm.hpp"

namespace filament_dryer {

class PWMFeedforwardControl : public IControlAlgorithm {
public:
    PWMFeedforwardControl();
    ~PWMFeedforwardControl() override;
    
    bool begin(const JsonObject& config) override;
    float compute(float target_temp, float current_temp, float dt) override;
    void reset() override;
    String getType() const override { return "pwm_feedforward"; }
    String getName() const override { return "PWM Feedforward"; }
    bool needsTuning() const override { return false; }
    JsonObject getParameters() override { return JsonObject(); }
    void setParameters(const JsonObject& params) override {}
    bool isInitialized() const override { return initialized_; }

private:
    float base_pwm_ = 50.0f;        // Base PWM at target temp
    float temp_coeff_ = 2.5f;       // PWM change per degree C
    float max_pwm_ = 100.0f;
    float min_pwm_ = 0.0f;
    bool initialized_ = false;
};

} // namespace filament_dryer