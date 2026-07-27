/**
 * GPIOActuator - Simple Digital GPIO On/Off Actuator
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class GPIOActuator : public IActuatorDriver {
public:
    GPIOActuator();
    ~GPIOActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "gpio"; }
    String getName() const override { return "Digital GPIO"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }

private:
    int8_t gpio_pin_ = -1;
    bool active_high_ = true;
    bool initialized_ = false;
    ActuatorState state_;

    void applyState(bool on);
};

} // namespace filament_dryer