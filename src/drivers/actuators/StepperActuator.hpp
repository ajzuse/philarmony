/**
 * StepperActuator - Stepper Motor Driver (A4988, DRV8825, TMC2209, etc.)
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class StepperActuator : public IActuatorDriver {
public:
    StepperActuator();
    ~StepperActuator() override;
    
    bool begin(const JsonObject& config) override;
    bool setPower(float power_pct) override;
    void emergencyStop() override;
    ActuatorState getState() const override;
    String getType() const override { return "stepper"; }
    String getName() const override { return "Stepper Motor"; }
    bool isHealthy() const override { return initialized_ && !state_.fault; }

    // Stepper-specific methods
    bool moveSteps(int32_t steps);
    bool setSpeed(float steps_per_sec);
    bool home();

private:
    int8_t step_pin_ = -1;
    int8_t dir_pin_ = -1;
    int8_t enable_pin_ = -1;
    int8_t ms1_pin_ = -1;
    int8_t ms2_pin_ = -1;
    int8_t ms3_pin_ = -1;
    uint32_t max_speed_ = 1000;
    float current_speed_ = 0;
    int32_t target_position_ = 0;
    int32_t current_position_ = 0;
    uint8_t microsteps_ = 16;
    bool initialized_ = false;
    ActuatorState state_;

    void setMicrostepping(uint8_t ms);
    void setDirection(bool cw);
    void stepPulse();
};

} // namespace filament_dryer