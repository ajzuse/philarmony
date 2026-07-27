/**
 * SSR Actuator Driver - Solid State Relay Control
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class SSRActuator : public IActuatorDriver {
public:
    SSRActuator();
    ~SSRActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "ssr"; }
    String getName() const override { return "Solid State Relay"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }

private:
    int8_t gpio_pin_ = 25;
    uint32_t pwm_freq_ = 1000;
    uint8_t pwm_channel_ = 0;
    uint8_t pwm_resolution_ = 10;
    uint8_t max_power_pct_ = 100;
    bool initialized_ = false;
    ActuatorState state_;
    
    void applyDuty(uint32_t duty);
};

} // namespace filament_dryer