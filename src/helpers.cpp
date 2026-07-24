/**
 * Helper functions and callbacks for main.cpp
 */
#include "main.cpp"

// ============================================================
// Callback Implementations
// ============================================================

void onStateChange(StateMachine::SystemState old_state, StateMachine::SystemState new_state) {
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, 
                     "State transition: %s -> %s", 
                     stateMachine.getStateName().c_str(), 
                     stateMachine.getStateName().c_str());
    
    // Broadcast state change via WebSocket
    JsonObject state_obj = wsServer.makeJsonObject();
    state_obj["old_state"] = old_state;
    state_obj["new_state"] = new_state;
    wsServer.broadcastTelemetry(state_obj);
}

void onSessionUpdate(const StateMachine::DryingSession& session) {
    // Build telemetry payload
    JsonObject telemetry = wsServer.makeJsonObject();
    telemetry["status"] = session.status;
    telemetry["chamber_temp_c"] = session.current_temp;
    telemetry["target_temp_c"] = session.target_temp_c;
    telemetry["humidity_pct"] = session.current_humidity;
    telemetry["target_humidity_pct"] = session.target_humidity_pct;
    telemetry["heater_on"] = session.heater_on;
    telemetry["heater_power_pct"] = session.heater_power_pct;
    telemetry["exhaust_fan_on"] = session.fan_on;
    telemetry["exhaust_fan_power_pct"] = session.fan_power_pct;
    telemetry["elapsed_time_sec"] = session.elapsed_sec;
    telemetry["remaining_time_sec"] = session.remaining_sec;
    telemetry["cpu_usage_pct"] = sysMetrics.getCpuUsage();
    telemetry["memory_free_bytes"] = ESP.getFreeHeap();
    telemetry["uptime_sec"] = millis() / 1000;
    
    wsServer.broadcastTelemetry(telemetry);
    
    // Log telemetry every 10 seconds
    static uint32_t last_log = 0;
    if (millis() - last_log >= 10000) {
        logMgr.logDryingTelemetry(
            session.current_temp, session.target_temp_c,
            session.current_humidity, session.target_humidity_pct,
            session.heater_power_pct, session.fan_power_pct,
            session.elapsed_sec, session.remaining_sec
        );
        last_log = millis();
    }
}

void onSafetyFault(FaultCode fault, const String& message) {
    logMgr.logDryingFault(String(fault), message);
    wsServer.broadcastFault(fault, message);
    
    // Force emergency stop on heater
    heaterActuator.emergencyStop();
    fanActuator.setPower(100.0f); // Full fan for cooling
}

void onLogCallback(const String& line, bool is_drying_log) {
    wsServer.broadcastLog(line, is_drying_log);
}

void onPidCalibrateProgress(int cycle, int total, float current_temp, 
                            float kp, float ki, float kd, bool done) {
    JsonObject progress = wsServer.makeJsonObject();
    progress["status"] = done ? "complete" : "calibrating";
    progress["cycle"] = cycle;
    progress["total_cycles"] = total;
    progress["current_temp_c"] = current_temp;
    progress["kp"] = kp;
    progress["ki"] = ki;
    progress["kd"] = kd;
    progress["saved_to_nvs"] = done;
    
    wsServer.broadcastPidCalibrate(progress);
}

void onPidCalibrateComplete(const PidAutotuneController::Result& result) {
    if (result.success) {
        // Save to NVS via ConfigManager
        ConfigManager::PidConfig pidCfg;
        pidCfg.kp = result.kp;
        pidCfg.ki = result.ki;
        pidCfg.kd = result.kd;
        pidCfg.calibrated = true;
        configMgr.setPidConfig(pidCfg);
        
        logMgr.logSystem(LogLevel::INFO, LogModule::PID, 
                         "PID auto-tune complete: Kp=%.2f, Ki=%.2f, Kd=%.2f",
                         result.kp, result.ki, result.kd);
    } else {
        logMgr.logSystem(LogLevel::ERROR, LogModule::PID, 
                         "PID auto-tune failed: %s", result.error.c_str());
    }
}

// ============================================================
// Driver Initialization
// ============================================================

void initializeSensors() {
    ConfigManager::SensorConfig sensorCfg = configMgr.getSensorConfig();
    
    // Initialize SHT31 (I2C)
    JsonObject sht31Config = sht31Sensor.makeJsonObject();
    sht31Config["type"] = "sht31";
    sht31Config["i2c_bus"] = sensorCfg.i2c_bus;
    sht31Config["sda_pin"] = sensorCfg.sda_pin;
    sht31Config["scl_pin"] = sensorCfg.scl_pin;
    sht31Config["i2c_address"] = sensorCfg.i2c_address;
    
    if (sht31Sensor.begin(sht31Config)) {
        activeTempSensor = &sht31Sensor;
        activeHumiditySensor = &sht31Sensor;
        logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, "SHT31 initialized");
    }
    
    // Initialize DHT22 if configured
    if (sensorCfg.type == "dht22" || sensorCfg.type == "dht22+sht31") {
        JsonObject dhtConfig = dht22Sensor.makeJsonObject();
        dhtConfig["gpio_pin"] = sensorCfg.gpio_pin;
        
        if (dht22Sensor.begin(dhtConfig)) {
            if (!activeTempSensor) activeTempSensor = &dht22Sensor;
            if (!activeHumiditySensor) activeHumiditySensor = &dht22Sensor;
            logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, "DHT22 initialized");
        }
    }
    
    // Initialize DS18B20 if configured
    if (sensorCfg.type == "ds18b20" || sensorCfg.type == "ds18b20+sht31") {
        JsonObject dsConfig = ds18b20Sensor.makeJsonObject();
        dsConfig["gpio_pin"] = sensorCfg.gpio_pin;
        dsConfig["resolution"] = 12;
        
        if (ds18b20Sensor.begin(dsConfig)) {
            if (!activeTempSensor) activeTempSensor = &ds18b20Sensor;
            logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, "DS18B20 initialized");
        }
    }
}

void initializeActuators() {
    ConfigManager::ActuatorConfig actuatorCfg = configMgr.getActuatorConfig();
    
    // Heater MOSFET AOD4184
    JsonObject heaterConfig = heaterActuator.makeJsonObject();
    heaterConfig["heater_pin"] = actuatorCfg.heater_pin;
    heaterConfig["heater_pwm_freq"] = actuatorCfg.heater_pwm_freq;
    heaterConfig["heater_max_power_pct"] = actuatorCfg.heater_max_power_pct;
    
    // Load PID config if calibrated
    ConfigManager::PidConfig pidCfg = configMgr.getPidConfig();
    if (pidCfg.calibrated) {
        heaterActuator.setPidConfig(pidCfg.kp, pidCfg.ki, pidCfg.kd);
    }
    
    if (heaterActuator.begin(heaterConfig)) {
        logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR, 
                         "AOD4184 Heater MOSFET initialized on GPIO %d", 
                         actuatorCfg.heater_pin);
    }
    
    // Fan actuator
    JsonObject fanConfig = fanActuator.makeJsonObject();
    fanConfig["fan_mode"] = actuatorCfg.fan_mode;
    fanConfig["fan_pin"] = actuatorCfg.fan_pin;
    fanConfig["fan_pwm_freq"] = actuatorCfg.fan_pwm_freq;
    fanConfig["cooldown_duration_sec"] = actuatorCfg.cooldown_duration_sec;
    
    if (fanActuator.begin(fanConfig)) {
        logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR, 
                         "Exhaust fan initialized (mode: %s)", 
                         actuatorCfg.fan_mode.c_str());
    }
}

void initializeDisplays() {
    ConfigManager::DisplayConfig displayCfg = configMgr.getDisplayConfig();
    
    if (!displayCfg.enabled) {
        logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, "Display disabled in config");
        return;
    }
    
    JsonObject displayConfig = st7789Display.makeJsonObject();
    displayConfig["driver"] = displayCfg.driver;
    displayConfig["bus_type"] = displayCfg.bus_type;
    displayConfig["width"] = displayCfg.width;
    displayConfig["height"] = displayCfg.height;
    displayConfig["rotation"] = displayCfg.rotation;
    displayConfig["spi_mosi"] = displayCfg.spi_mosi;
    displayConfig["spi_sclk"] = displayCfg.spi_sclk;
    displayConfig["spi_cs"] = displayCfg.spi_cs;
    displayConfig["dc_pin"] = displayCfg.dc_pin;
    displayConfig["rst_pin"] = displayCfg.rst_pin;
    displayConfig["backlight_pin"] = displayCfg.backlight_pin;
    displayConfig["fields"] = displayCfg.fields;
    
    // Try ST7789 first (LilyGo T-Display)
    if (displayCfg.driver == "st7789" || displayCfg.driver == "auto") {
        if (st7789Display.begin(displayConfig)) {
            logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, "ST7789 display initialized");
            return;
        }
    }
    
    // Try ILI9341 (CYD board)
    if (displayCfg.driver == "ili9341" || displayCfg.driver == "auto") {
        if (ili9341Display.begin(displayConfig)) {
            logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, "ILI9341 display initialized");
            return;
        }
    }
    
    // Try SSD1306 (OLED I2C)
    if (displayCfg.driver == "ssd1306" || displayCfg.driver == "auto") {
        if (ssd1306Display.begin(displayConfig)) {
            logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, "SSD1306 display initialized");
            return;
        }
    }
    
    logMgr.logSystem(LogLevel::WARNING, LogModule::DISPLAY, "No display driver initialized");
}

void initializeNetwork() {
    // Start WiFi manager
    if (wifiMgr.begin()) {
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK, "WiFi manager started");
    }
    
    // Start WebServer (HTTP)
    if (webServer.begin(&configMgr, &logMgr)) {
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK, "HTTP server started on port 80");
    }
    
    // Start WebSocket server
    if (wsServer.begin()) {
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK, "WebSocket server started");
    }
}

// ============================================================
// FreeRTOS Tasks
// ============================================================

void controlLoopTask(void* pvParameters) {
    const TickType_t interval = pdMS_TO_TICKS(1000); // 1Hz control loop
    TickType_t last_wake = xTaskGetTickCount();
    
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, "Control loop started on Core 1");
    
    while (true) {
        vTaskDelayUntil(&last_wake, interval);
        
        uint32_t loop_start = millis();
        
        // Read sensors
        float chamber_temp = NAN;
        float humidity = NAN;
        
        if (activeTempSensor) {
            SensorReading temp_reading = activeTempSensor->read();
            if (temp_reading.valid) {
                chamber_temp = temp_reading.temperature;
            }
        }
        
        if (activeHumiditySensor) {
            SensorReading hum_reading = activeHumiditySensor->read();
            if (hum_reading.valid) {
                humidity = hum_reading.humidity;
            }
        }
        
        // Safety check
        bool sensor_ok = !isnan(chamber_temp);
        uint8_t heater_power = stateMachine.getCurrentSession().heater_power_pct;
        bool heater_on = stateMachine.getCurrentSession().heater_on;
        
        if (!safetyEngine.checkSafety(chamber_temp, 
                                       stateMachine.getCurrentSession().target_temp_c,
                                       heater_power, heater_on, sensor_ok)) {
            // Safety fault - emergency stop already triggered
            continue;
        }
        
        // PID control if in DRYING state
        if (stateMachine.isDrying()) {
            float target_temp = stateMachine.getCurrentSession().target_temp_c;
            
            if (!isnan(chamber_temp)) {
                // Compute PID output
                float pid_output = heaterActuator.computePid(target_temp, chamber_temp, 1.0f);
                heaterActuator.setPower(pid_output);
            }
            
            // Fan control - run during heating
            if (stateMachine.getCurrentSession().heater_on) {
                fanActuator.setPower(80.0f); // 80% fan during heating
            } else {
                fanActuator.setPower(0.0f);
            }
        }
        
        // Update state machine
        stateMachine.updateDryingProgress(chamber_temp, humidity,
                                           stateMachine.getCurrentSession().heater_power_pct,
                                           stateMachine.getCurrentSession().heater_on,
                                           stateMachine.getCurrentSession().fan_power_pct,
                                           stateMachine.getCurrentSession().fan_on);
        
        // Update system metrics
        sysMetrics.update();
        
        // Log loop timing
        uint32_t loop_time = millis() - loop_start;
        if (loop_time > 1200) {
            logMgr.logSystem(LogLevel::WARNING, LogModule::SYSTEM, 
                             "Control loop overrun: %lums", loop_time);
        }
    }
}

void networkTask(void* pvParameters) {
    const TickType_t interval = pdMS_TO_TICKS(100); // 10Hz network loop
    TickType_t last_wake = xTaskGetTickCount();
    
    logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK, "Network task started on Core 0");
    
    while (true) {
        vTaskDelayUntil(&last_wake, interval);
        
        // Process WiFi events
        wifiMgr.loop();
        
        // WebSocket cleanup is automatic in AsyncWebServer
        // WebSocket ping/pong handled by library
    }
}

void displayTask(void* pvParameters) {
    const TickType_t interval = pdMS_TO_TICKS(1000); // 1Hz display update
    TickType_t last_wake = xTaskGetTickCount();
    
    logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, "Display task started on Core 0");
    
    while (true) {
        vTaskDelayUntil(&last_wake, interval);
        
        // Update display if enabled and initialized
        // The actual display drivers are updated in their respective classes
        // This task just ensures periodic refresh if needed
    }
}