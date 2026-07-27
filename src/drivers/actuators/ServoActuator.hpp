/**
 * ServoActuator - Servo Motor Driver
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class ServoActuator : public IActuatorDriver {
public:
    ServoActuator();
    ~ServoActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "servo"; }
    String getName() const override { return "Servo Motor"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }

private:
    int8_t pwm_pin_ = -1;
    int8_t enable_pin_ = -1;
    uint32_t pwm_freq_ = 50;
    uint8_t pwm_channel_ = 0;
    uint8_t pwm_resolution_ = 12;
    uint16_t min_pulse_ = 500;
    uint16_t max_pulse_ = 2500;
    float min_angle_ = 0;
    float max_angle_ = 180;
    bool initialized_ = false;
    ActuatorState state_;

    uint16_t angleToPulse(float angle) const;
    void applyPulse(uint16_t pulse_us);
};

} // namespace filament_dryer