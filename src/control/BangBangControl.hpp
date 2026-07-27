/**
 * BangBangControl - Hysteresis-based on/off control
 */
#pragma once

#include "IControlAlgorithm.hpp"

namespace filament_dryer {

struct BangBangConfig {
    float hysteresis = 1.0f;  // degrees C
    float min_cycle_time = 10.0f;  // seconds
};

class BangBangControl : public IControlAlgorithm {
public:
    BangBangControl();
    ~BangBangControl() override;
    
    bool begin(const JsonObject& config) override;
    float compute(float target, float current, float dt) override;
    void reset() override;
    String getType() const override { return "bang_bang"; }
    String getName() const override { return "Bang-Bang (Hysteresis)"; }
    bool needsTuning() const override { return false; }
    JsonObject getParameters() override;
    void setParameters(const JsonObject& params) override;
    bool isInitialized() const override { return initialized_; }

private:
    BangBangConfig config_;
    float last_output_ = 0.0f;
    uint32_t last_switch_time_ = 0;
    bool initialized_ = false;
    
    bool shouldSwitch(float target, float current);
};

} // namespace filament_dryer