/**
 * SharedMosfetActuator - Heater and Fan sharing same MOSFET output
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class SharedMosfetActuator : public IActuatorDriver {
public:
    SharedMosfetActuator();
    ~SharedMosfetActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "shared_mosfet"; }
    String getName() const override { return "Shared MOSFET (Heater+Fan)"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }

    // Separate control for heater and fan
    bool setHeaterPower(float power_pct);
    bool setFanPower(float power_pct);

private:
    int8_t pwm_pin_ = -1;
    uint32_t pwm_freq_ = 1000;
    uint8_t pwm_channel_ = 0;
    uint8_t pwm_resolution_ = 10;
    uint8_t max_power_pct_ = 100;
    bool initialized_ = false;
    ActuatorState state_;
    float heater_power_ = 0;
    float fan_power_ = 0;

    void applyPWM(float power_pct);
};

} // namespace filament_dryer