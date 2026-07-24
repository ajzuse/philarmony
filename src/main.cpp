/**
 * Filament Dryer ESP32 - Main Firmware Entry Point
 * 
 * Architecture:
 * - Core 0: Network (WiFi, WebServer, WebSocket, HTTP Logs)
 * - Core 1: Control Loop (Sensors, PID, Safety, Display)
 * 
 * Klipper-inspired object architecture with plugin system
 */
#include <Arduino.h>
#include "firmware_version.h"
#include "ConfigManager.hpp"
#include "StateMachine.hpp"
#include "LogManager.hpp"
#include "SafetyEngine.hpp"
#include "PidAutotuneController.hpp"
#include "WifiManager.hpp"
#include "WebServer.hpp"
#include "WebSocketServer.hpp"
#include "SystemMetrics.hpp"
#include "drivers/interfaces/IDriverInterfaces.hpp"
#include "drivers/sensors/SHT31Sensor.hpp"
#include "drivers/sensors/DHT22Sensor.hpp"
#include "drivers/sensors/DS18B20Sensor.hpp"
#include "drivers/actuators/MosfetActuator.hpp"
#include "drivers/actuators/FanActuator.hpp"
#include "drivers/display/ST7789Display.hpp"
#include "drivers/display/ILI9341Display.hpp"
#include "drivers/display/SSD1306Display.hpp"
#include "plugins/PluginManager.hpp"

using namespace filament_dryer;

// ============================================================
// Global Objects (Klipper-style Object Registry Pattern)
// ============================================================
ConfigManager configMgr;
StateMachine stateMachine;
LogManager logMgr;
SafetyEngine safetyEngine;
PidAutotuneController pidAutotune;
WifiManager wifiMgr;
WebServer webServer(80);
WebSocketServer wsServer;
SystemMetrics sysMetrics;

// Driver instances
SHT31Sensor sht31Sensor;
DHT22Sensor dht22Sensor;
DS18B20Sensor ds18b20Sensor;
MosfetActuator heaterActuator;
FanActuator fanActuator;
ST7789Display st7789Display;
ILI9341Display ili9341Display;
SSD1306Display ssd1306Display;
PluginManager pluginMgr;

// Current active sensor driver
ISensorDriver* activeTempSensor = nullptr;
ISensorDriver* activeHumiditySensor = nullptr;

// ============================================================
// FreeRTOS Task Handles
// ============================================================
TaskHandle_t controlLoopTaskHandle = nullptr;
TaskHandle_t networkTaskHandle = nullptr;
TaskHandle_t displayTaskHandle = nullptr;

// ============================================================
// Control Loop (Core 1) - 50Hz = 20ms period
// ============================================================
void controlLoopTask(void* pvParameters) {
    const TickType_t period = pdMS_TO_TICKS(20); // 50Hz
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    // PID timing
    uint32_t lastPidTime = millis();
    
    for (;;) {
        vTaskDelayUntil(&lastWakeTime, period);
        
        uint32_t now = millis();
        float dt = (now - lastPidTime) / 1000.0f;
        lastPidTime = now;
        
        // Feed safety watchdog
        safetyEngine.feedWatchdog();
        
        // Read active sensors
        SensorReading tempReading, humidityReading;
        if (activeTempSensor) {
            tempReading = activeTempSensor->read();
        }
        if (activeHumiditySensor) {
            humidityReading = activeHumiditySensor->read();
        }
        
        float chamberTemp = tempReading.valid ? tempReading.temperature : NAN;
        float chamberHumidity = humidityReading.valid ? humidityReading.humidity : NAN;
        
        // Safety check (runs every control loop iteration)
        uint8_t heaterPower = heaterActuator.getState().enabled ? 
                             (uint8_t)heaterActuator.getState().power_pct : 0;
        
        bool sensorConnected = tempReading.valid;
        if (!safetyEngine.checkSafety(chamberTemp, 
                                       stateMachine.getCurrentSession().target_temp_c,
                                       heaterPower, sensorConnected)) {
            // Safety fault - state machine will handle transition
            stateMachine.stopDrying(DryingStopReason::SENSOR_ERROR);
            continue;
        }
        
        // Update PID auto-tune if running
        if (pidAutotune.isRunning()) {
            pidAutotune.update(chamberTemp, heaterPower);
        }
        
        // Drying cycle control logic
        if (stateMachine.isDrying()) {
            DryingSession& session = stateMachine.getCurrentSession();
            
            // Check completion conditions
            bool stop = false;
            DryingStopReason reason = DryingStopReason::USER_STOPPED;
            
            if (session.max_duration_min > 0) {
                uint32_t elapsed_min = (millis() - session.start_timestamp) / 60000;
                if (elapsed_min >= session.max_duration_min) {
                    stop = true;
                    reason = DryingStopReason::MAX_TIME;
                }
            }
            
            if (!isnan(session.target_humidity_pct) && 
                chamberHumidity <= session.target_humidity_pct) {
                stop = true;
                reason = DryingStopReason::HUMIDITY_REACHED;
            }
            
            if (!isnan(chamberTemp) && chamberTemp >= 80.0f) {
                stop = true;
                reason = DryingStopReason::OVER_TEMP;
            }
            
            if (!sensorConnected) {
                stop = true;
                reason = DryingStopReason::SENSOR_ERROR;
            }
            
            if (stop) {
                stateMachine.stopDrying(reason);
                continue;
            }
            
            // PID Control
            float targetTemp = session.target_temp_c;
            float pidOutput = 0.0f;
            
            // Use auto-tuned PID if available, else fallback
            PidConfig pidCfg = configMgr.getPidConfig();
            if (pidCfg.calibrated && pidCfg.kp > 0) {
                float error = targetTemp - chamberTemp;
                static float integral = 0;
                static float lastError = 0;
                float dt = 0.02f; // 20ms
                
                integral = constrain(integral + error * dt, -1000, 1000);
                float derivative = (error - lastError) / dt;
                
                pidOutput = pidCfg.kp * error + pidCfg.ki * integral + pidCfg.kd * derivative;
                lastError = error;
            } else {
                // Simple bang-bang with hysteresis as fallback
                if (chamberTemp < targetTemp - 1.0f) pidOutput = 100.0f;
                else if (chamberTemp > targetTemp + 0.5f) pidOutput = 0.0f;
            }
            
            // Apply power with soft limits
            pidOutput = constrain(pidOutput, 0.0f, 100.0f);
            heaterActuator.setPower(pidOutput);
            
            // Fan control based on heater state
            if (pidOutput > 0 || chamberTemp > 40.0f) {
                fanActuator.setPower(80.0f);
            } else {
                fanActuator.setPower(0.0f);
            }
        }
        
        // Update state machine
        stateMachine.updateDryingProgress(chamberTemp, chamberHumidity,
                                          heaterActuator.getState().power_pct,
                                          heaterActuator.getState().enabled,
                                          fanActuator.getState().power_pct,
                                          fanActuator.getState().enabled);
        
        // Log telemetry (every 10 iterations = 1Hz)
        static int logCounter = 0;
        if (++logCounter >= 50) { // 50 * 20ms = 1s
            logCounter = 0;
            
            if (stateMachine.isDrying()) {
                logMgr.logDryingTelemetry(chamberTemp, 
                    stateMachine.getCurrentSession().target_temp_c,
                    chamberHumidity,
                    stateMachine.getCurrentSession().target_humidity_pct,
                    heaterActuator.getState().power_pct,
                    fanActuator.getState().power_pct,
                    stateMachine.getCurrentSession().elapsed_sec,
                    stateMachine.getCurrentSession().remaining_sec);
            }
            
            // Broadcast via WebSocket
            StaticJsonDocument<512> telemetry;
            telemetry["status"] = stateMachine.getStateName();
            telemetry["chamber_temp_c"] = chamberTemp;
            telemetry["target_temp_c"] = stateMachine.getCurrentSession().target_temp_c;
            telemetry["humidity_pct"] = chamberHumidity;
            telemetry["target_humidity_pct"] = stateMachine.getCurrentSession().target_humidity_pct;
            telemetry["heater_on"] = heaterActuator.getState().enabled;
            telemetry["heater_power_pct"] = heaterActuator.getState().power_pct;
            telemetry["exhaust_fan_on"] = fanActuator.getState().enabled;
            telemetry["exhaust_fan_power_pct"] = fanActuator.getState().power_pct;
            telemetry["elapsed_time_sec"] = stateMachine.getCurrentSession().elapsed_sec;
            telemetry["remaining_time_sec"] = stateMachine.getCurrentSession().remaining_sec;
            telemetry["cpu_usage_pct"] = sysMetrics.getCpuUsage();
            telemetry["memory_free_bytes"] = sysMetrics.getFreeHeap();
            telemetry["uptime_sec"] = millis() / 1000;
            
            wsServer.broadcastTelemetry(telemetry);
        }
    }
}

// ============================================================
// Network Task (Core 0) - Handles WiFi, WebServer, WebSocket
// ============================================================
void networkTask(void* pvParameters) {
    const TickType_t period = pdMS_TO_TICKS(10); // 100Hz
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        vTaskDelayUntil(&lastWakeTime, period);
        
        // WiFi management
        wifiMgr.loop();
        
        // WebSocket cleanup
        wsServer.cleanupClients();
        
        // Log streaming to WebSocket
        static uint32_t lastLogStream = 0;
        if (millis() - lastLogStream > 1000) {
            lastLogStream = millis();
            // Log streaming handled by LogManager callback
        }
    }
}

// ============================================================
// Display Task (Core 0) - Independent rendering
// ============================================================
void displayTask(void* pvParameters) {
    const TickType_t period = pdMS_TO_TICKS(1000); // 1Hz default
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        vTaskDelayUntil(&lastWakeTime, period);
        
        DisplayConfig dispConfig = configMgr.getDisplayConfig();
        if (!dispConfig.enabled) continue;
        
        // Build status fields for display
        JsonObject statusFields = wsServer.makeJsonObject();
        DryingSession session = stateMachine.getCurrentSession();
        
        statusFields["chamber_temp_c"] = session.target_temp_c; // Will be filled by control loop
        statusFields["target_temp_c"] = session.target_temp_c;
        statusFields["humidity_pct"] = session.target_humidity_pct;
        statusFields["heater_power_pct"] = 0; // Filled by control loop
        telemetry["status"] = stateMachine.getStateName();
        
        // Update active display
        if (ssd1306Display.isConnected()) {
            ssd1306Display.update(statusFields);
        } else if (st7789Display.isConnected()) {
            st7789Display.update(statusFields);
        } else if (ili9341Display.isConnected()) {
            ili9341Display.update(statusFields);
        }
    }
}

// ============================================================
// Callbacks
// ============================================================
void onStateChange(SystemState oldState, SystemState newState) {
    String oldStr = stateMachine.getStateName();
    String newStr = StateMachine::getStateName(newState);
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, 
                     "State transition: %s -> %s", oldStr.c_str(), newStr.c_str());
    
    // On fault, trigger emergency stop on actuators
    if (newState == SystemState::FAULT_STOPPED) {
        heaterActuator.emergencyStop();
        fanActuator.emergencyStop();
    }
}

void onSessionUpdate(const DryingSession& session) {
    // Update display, log, etc.
}

void onSafetyFault(FaultCode fault, const String& message) {
    logMgr.logSystem(LogLevel::ERROR, LogModule::SAFETY, "%s", message.c_str());
    wsServer.broadcastFault(fault, message);
    
    // Force state machine to fault state
    stateMachine.transitionTo(SystemState::FAULT_STOPPED);
}

void onPidCalibrateProgress(int cycle, int total, float temp, float kp, float ki, float kd, bool done) {
    StaticJsonDocument<256> doc;
    doc["status"] = done ? "complete" : "calibrating";
    doc["cycle"] = cycle;
    doc["total_cycles"] = total;
    doc["current_temp_c"] = temp;
    doc["kp"] = kp;
    doc["ki"] = ki;
    doc["kd"] = kd;
    doc["saved_to_nvs"] = done;
    
    wsServer.broadcastPidCalibrate(doc);
}

void onPidCalibrateComplete(const PidAutotuneController::Result& result) {
    if (result.success) {
        // Save to NVS
        PidConfig cfg = {result.kp, result.ki, result.kd, true};
        configMgr.setPidConfig(cfg);
        
        logMgr.logSystem(LogLevel::INFO, LogModule::PID, 
                         "PID auto-tune complete: Kp=%.2f, Ki=%.2f, Kd=%.2f",
                         result.kp, result.ki, result.kd);
    } else {
        logMgr.logSystem(LogLevel::ERROR, LogModule::PID, 
                         "PID auto-tune failed: %s", result.error.c_str());
    }
    
    // Notify via WebSocket
    StaticJsonDocument<256> doc;
    doc["status"] = result.success ? "success" : "error";
    if (result.success) {
        doc["kp"] = result.kp;
        doc["ki"] = result.ki;
        doc["kd"] = result.kd;
    } else {
        doc["error"] = result.error;
    }
    wsServer.broadcastPidCalibrate(doc);
}

void onLogCallback(const String& line, bool isDryingLog) {
    wsServer.broadcastLog(line, isDryingLog);
}

// ============================================================
// Setup & Initialization
// ============================================================
void initializeSensors() {
    JsonObject sensorConfig = configMgr.getSensorConfig();
    
    // Initialize based on configured type
    String type = sensorConfig["type"] | "sht31";
    
    if (type == "sht31" || type == "sht30") {
        if (sht31Sensor.begin(sensorConfig)) {
            activeTempSensor = &sht31Sensor;
            activeHumiditySensor = &sht31Sensor;
            logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, 
                             "SHT31 initialized on I2C 0x%02X", 
                             sensorConfig["i2c_address"] | 0x44);
        }
    } else if (type == "dht22") {
        if (dht22Sensor.begin(sensorConfig)) {
            activeTempSensor = &dht22Sensor;
            activeHumiditySensor = &dht22Sensor;
            logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, 
                             "DHT22 initialized on GPIO %d", 
                             sensorConfig["gpio_pin"] | 4);
        }
    } else if (type == "ds18b20") {
        if (ds18b20Sensor.begin(sensorConfig)) {
            activeTempSensor = &ds18b20Sensor;
            logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, 
                             "DS18B20 initialized on GPIO %d", 
                             sensorConfig["gpio_pin"] | 4);
        }
    }
    
    // Fallback: try auto-detection if configured
    if (!activeTempSensor && type == "auto") {
        // Try SHT31 first
        if (sht31Sensor.begin(sensorConfig)) {
            activeTempSensor = &sht31Sensor;
            activeHumiditySensor = &sht31Sensor;
        } 
        // Try DS18B20
        else if (ds18b20Sensor.begin(sensorConfig)) {
            activeTempSensor = &ds18b20Sensor;
        }
        // Try DHT22
        else if (dht22Sensor.begin(sensorConfig)) {
            activeTempSensor = &dht22Sensor;
            activeHumiditySensor = &dht22Sensor;
        }
    }
    
    if (!activeTempSensor) {
        logMgr.logSystem(LogLevel::WARNING, LogModule::SENSOR, 
                         "No temperature sensor detected!");
    }
}

void initializeActuators() {
    JsonObject actuatorConfig = configMgr.getActuatorConfig();
    
    // Heater MOSFET AOD4184
    if (heaterActuator.begin(actuatorConfig)) {
        // Set PID config if available
        PidConfig pidCfg = configMgr.getPidConfig();
        if (pidCfg.calibrated) {
            heaterActuator.setPidConfig(pidCfg.kp, pidCfg.ki, pidCfg.kd);
        }
        
        logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR, 
                         "Heater MOSFET AOD4184 on GPIO %d", 
                         actuatorConfig["heater_pin"] | 25);
    }
    
    // Exhaust Fan
    if (fanActuator.begin(actuatorConfig)) {
        logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR, 
                         "Exhaust fan on GPIO %d (mode: %s)", 
                         actuatorConfig["fan_pin"] | 26,
                         actuatorConfig["fan_mode"] | "independent_pwm");
    }
}

void initializeDisplays() {
    JsonObject dispConfig = configMgr.getDisplayConfig();
    
    if (!dispConfig["enabled"] | false) return;
    
    String driver = dispConfig["driver"] | "auto";
    bool initialized = false;
    
    if (driver == "st7789" || driver == "auto") {
        if (st7789Display.begin(dispConfig)) {
            initialized = true;
        }
    }
    
    if (!initialized && (driver == "ili9341" || driver == "auto")) {
        if (ili9341Display.begin(dispConfig)) {
            initialized = true;
        }
    }
    
    if (!initialized && (driver == "ssd1306" || driver == "auto")) {
        if (ssd1306Display.begin(dispConfig)) {
            initialized = true;
        }
    }
    
    if (initialized) {
        logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY, 
                         "Display %s initialized (%dx%d)", 
                         driver.c_str(),
                         dispConfig["width"] | 128,
                         dispConfig["height"] | 64);
    }
}

void initializeNetwork() {
    // Start WiFi manager
    wifiMgr.begin();
    
    // Register callbacks
    wifiMgr.setStatusCallback([](WifiManager::Status s) {
        if (s == WifiManager::Status::CONNECTED) {
            logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK, 
                             "WiFi connected: %s", wifiMgr.getLocalIP().c_str());
        }
    });
    
    // Start WebServer
    if (webServer.begin(&configMgr, &logMgr)) {
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK, 
                         "HTTP server started on port 80");
    }
    
    // Start WebSocket server
    if (wsServer.begin(&configMgr, &stateMachine, &heaterActuator, &fanActuator, &pidAutotune)) {
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK, 
                         "WebSocket server started on ws://<ip>/ws");
    }
}

void setup() {
    // Serial for debugging
    Serial.begin(115200);
    delay(100);
    
    Serial.printf("\n\n=== %s v%s ===\n", FIRMWARE_NAME, FIRMWARE_VERSION);
    Serial.printf("Build: %s\n", FIRMWARE_BUILD_DATE);
    Serial.printf("Chip: %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    
    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("ERROR: LittleFS mount failed!");
    }
    
    // Initialize ConfigManager (loads all NVS configs)
    if (!configMgr.begin()) {
        Serial.println("ERROR: ConfigManager init failed");
    }
    
    // Initialize core subsystems
    if (!stateMachine.begin()) {
        Serial.println("ERROR: StateMachine init failed");
    }
    
    if (!logMgr.begin()) {
        Serial.println("ERROR: LogManager init failed");
    }
    
    SafetyEngine::SafetyConfig safetyCfg;
    safetyCfg.hard_temp_limit_c = 80.0f;
    safetyCfg.watchdog_enabled = true;
    if (!safetyEngine.begin(safetyCfg)) {
        Serial.println("ERROR: SafetyEngine init failed");
    }
    
    // Setup callbacks
    stateMachine.setStateChangeCallback(onStateChange);
    stateMachine.setSessionUpdateCallback(onSessionUpdate);
    safetyEngine.setFaultCallback(onSafetyFault);
    pidAutotune.setHeaterCallback([](float power) { 
        // This will be called by PID autotune during calibration
    });
    pidAutotune.setProgressCallback(onPidCalibrateProgress);
    pidAutotune.setCompleteCallback(onPidCalibrateComplete);
    logMgr.setLogCallback(onLogCallback);
    
    // Initialize PID auto-tune with heater callback
    pidAutotune.setHeaterCallback([](float power) {
        heaterActuator.setPower(power);
    });
    
    // Initialize drivers
    initializeSensors();
    initializeActuators();
    initializeDisplays();
    
    // Initialize system metrics
    sysMetrics.begin();
    
    // Initialize network (WiFi, WebServer, WebSocket)
    initializeNetwork();
    
    // Start FreeRTOS tasks
    xTaskCreatePinnedToCore(
        controlLoopTask, "ControlLoop", 8192, nullptr, 5, 
        &controlLoopTaskHandle, 1); // Core 1
    
    xTaskCreatePinnedToCore(
        networkTask, "NetworkTask", 8192, nullptr, 3, 
        &networkTaskHandle, 0); // Core 0
    
    xTaskCreatePinnedToCore(
        displayTask, "DisplayTask", 4096, nullptr, 2, 
        &displayTaskHandle, 0); // Core 0
    
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, 
                     "Filament Dryer ESP32 v%s started", FIRMWARE_VERSION);
    
    Serial.println("=== Initialization Complete ===");
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
}

void loop() {
    // Main loop is empty - all work done in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}