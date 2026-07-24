/**
 * MOSFET Actuator Driver for AOD4184 Heater Control
 */
#pragma once

#include "IDriverInterfaces.hpp"

namespace filament_dryer {

class MosfetActuator : public IActuatorDriver {
public:
    MosfetActuator();
    ~MosfetActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "mosfet_aod4184"; }
    String getName() const override { return "AOD4184 Heater MOSFET"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }
    
    // PID integration
    void setPidConfig(float kp, float ki, float kd);
    float computePid(float target_temp, float current_temp, float dt);
    
private:
    int8_t gpio_pin_ = 25;
    uint32_t pwm_freq_ = 1000;
    uint8_t pwm_channel_ = 0;
    uint8_t pwm_resolution_ = 10;
    uint8_t max_power_pct_ = 100;
    bool initialized_ = false;
    ActuatorState state_;
    
    // PID state
    float pid_kp_ = 0.0f;
    float pid_ki_ = 0.0f;
    float pid_kd_ = 0.0f;
    float pid_integral_ = 0.0f;
    float pid_last_error_ = 0.0f;
    uint32_t pid_last_time_ = 0;
    bool pid_enabled_ = false;
    
    float power_to_duty(float power_pct) const;
    void applyDuty(uint32_t duty);
    void resetPid();
};

} // namespace filament_dryer