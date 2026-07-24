/**
 * PidAutotuneController - Ziegler-Nichols Relay Auto-Tuning
 * Inspired by Klipper's PID_CALIBRATE
 */
#pragma once

#include <Arduino.h>

namespace filament_dryer {

class PidAutotuneController {
public:
    struct Config {
        float target_temp = 50.0f;
        int max_cycles = 5;
        float pwm_step = 100.0f;      // 0-100% PWM drive
        float max_temp = 80.0f;       // Safety limit
        int cycle_timeout_sec = 300;  // Per cycle timeout
    };
    
    struct Result {
        bool success = false;
        float kp = 0.0f;
        float ki = 0.0f;
        float kd = 0.0f;
        String error;
    };
    
    using ProgressCallback = void(*)(int cycle, int total, float current_temp, 
                                     float kp, float ki, float kd, bool done);
    using CompleteCallback = void(*)(const Result& result);
    
    PidAutotuneController();
    ~PidAutotuneController();
    
    bool startCalibration(const Config& config, 
                          ProgressCallback progress_cb,
                          CompleteCallback complete_cb);
    void stopCalibration();
    void update(float current_temp, float heater_pwm_pct);
    bool isRunning() const { return running_; }
    const Result& getResult() const { return result_; }

private:
    enum State {
        IDLE,
        HEATING_UP,
        COOLING_DOWN,
        MEASURING_OSCILLATION,
        CALCULATING,
        COMPLETE,
        ERROR
    };
    
    State state_ = IDLE;
    Config config_;
    Result result_;
    ProgressCallback progress_cb_ = nullptr;
    CompleteCallback complete_cb_ = nullptr;
    bool running_ = false;
    
    // Ziegler-Nichols measurement
    float cycle_start_temp_ = 0.0f;
    float peak_temp_ = 0.0f;
    float valley_temp_ = 0.0f;
    uint32_t cycle_start_time_ = 0;
    uint32_t last_peak_time_ = 0;
    uint32_t last_valley_time_ = 0;
    bool heating_phase_ = true;
    int cycles_completed_ = 0;
    float oscillation_period_ = 0.0f;
    float oscillation_amplitude_ = 0.0f;
    
    // Callbacks
    using HeaterSetCallback = void(*)(float power_pct);
    HeaterSetCallback heater_cb_ = nullptr;
    
    void reset();
    void transitionTo(State new_state);
    void checkSafety(float current_temp);
    void calculatePid();
    
public:
    void setHeaterCallback(HeaterSetCallback cb) { heater_cb_ = cb; }
};

} // namespace filament_dryer