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
#include <LittleFS.h>
#include <cmath>
#include "firmware_version.h"
#include "core/ConfigManager.hpp"
#include "core/StateMachine.hpp"
#include "core/LogManager.hpp"
#include "core/SafetyEngine.hpp"
#include "core/PidAutotuneController.hpp"
#include "core/ProfileManager.hpp"
#include "core/HardwareConfigParser.hpp"
#include "core/DriverRegistry.hpp"
#include "control/ControlEngine.hpp"
#include "network/WifiManager.hpp"
#include "network/WebServer.hpp"
#include "network/WebSocketServer.hpp"
#include "utils/SystemMetrics.hpp"
#include "drivers/interfaces/IDriverInterfaces.hpp"
#include "drivers/display/DisplayManager.hpp"
#include "plugins/IPlugin.hpp"

using namespace filament_dryer;

// ============================================================
// Global Objects (Klipper-style Object Registry Pattern)
// ============================================================
ConfigManager configMgr;
StateMachine stateMachine;
LogManager logMgr;
SafetyEngine safetyEngine;
PidAutotuneController pidAutotune;
HardwareConfigParser hwParser;
ProfileManager profileMgr(configMgr);
ControlEngine controlEngine;
DisplayManager displayManager;
PluginManager pluginMgr;
WifiManager wifiMgr;
WebServer webServer(80);
WebSocketServer wsServer;
SystemMetrics sysMetrics;

ISensorDriver* activeTempSensor = nullptr;
ISensorDriver* activeHumiditySensor = nullptr;
IActuatorDriver* heaterActuator = nullptr;
IActuatorDriver* fanActuator = nullptr;
IDisplayDriver* activeDisplay = nullptr;

static bool cooldownActive = false;
static uint32_t cooldownEndMs = 0;
static DryingStopReason pendingCooldownReason = DryingStopReason::COMPLETED;

static SensorReading applySensorCalibration(const SensorReading& reading) {
    if (!reading.valid) {
        return reading;
    }
    SensorConfig cfg = configMgr.getSensorConfig();
    SensorReading calibrated = reading;
    if (!isnan(calibrated.temperature)) {
        calibrated.temperature =
            calibrated.temperature * cfg.temperature_scale + cfg.temperature_offset;
    }
    if (!isnan(calibrated.humidity)) {
        calibrated.humidity =
            calibrated.humidity * cfg.humidity_scale + cfg.humidity_offset;
    }
    return calibrated;
}

static void beginCooldown(DryingStopReason reason) {
    pendingCooldownReason = reason;
    cooldownActive = true;
    ActuatorConfig actuatorCfg = configMgr.getActuatorConfig();
    cooldownEndMs = millis() + (actuatorCfg.cooldown_duration_sec * 1000UL);
    if (heaterActuator) {
        heaterActuator->setPower(0.0f);
    }
    if (fanActuator) {
        fanActuator->setPower(80.0f);
    }
}

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
            tempReading = applySensorCalibration(activeTempSensor->read());
        }
        if (activeHumiditySensor) {
            humidityReading = applySensorCalibration(activeHumiditySensor->read());
        }
        
        float chamberTemp = tempReading.valid ? tempReading.temperature : NAN;
        float chamberHumidity = humidityReading.valid ? humidityReading.humidity : NAN;

        if (cooldownActive) {
            if (heaterActuator) heaterActuator->setPower(0.0f);
            if (fanActuator) fanActuator->setPower(80.0f);
            if (millis() >= cooldownEndMs) {
                cooldownActive = false;
                if (fanActuator) fanActuator->setPower(0.0f);
                pluginMgr.callOnSessionStop(stateMachine.getCurrentSession(), pendingCooldownReason);
            }
            continue;
        }
        
        float heaterPowerPct = (heaterActuator && heaterActuator->getState().enabled)
                                   ? heaterActuator->getState().power_pct
                                   : 0.0f;
        uint8_t heaterPower = static_cast<uint8_t>(heaterPowerPct);

        bool sensorConnected = tempReading.valid;
        if (!safetyEngine.checkSafety(chamberTemp,
                                       stateMachine.getCurrentSession().target_temp_c,
                                       heaterPower, heaterPower > 0, sensorConnected)) {
            stateMachine.stopDrying(DryingStopReason::SENSOR_ERROR);
            if (heaterActuator) heaterActuator->setPower(0.0f);
            if (fanActuator) fanActuator->setPower(0.0f);
            continue;
        }

        if (pidAutotune.isRunning()) {
            pidAutotune.update(chamberTemp, heaterPower);
        }

        if (stateMachine.isDrying()) {
            DryingSession& session = stateMachine.getCurrentSession();

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
                reason = DryingStopReason::SAFETY_CUTOFF;
            }

            if (!sensorConnected) {
                stop = true;
                reason = DryingStopReason::SENSOR_ERROR;
            }

            if (stop) {
                stateMachine.stopDrying(reason);
                if (reason == DryingStopReason::HUMIDITY_REACHED ||
                    reason == DryingStopReason::MAX_TIME ||
                    reason == DryingStopReason::COMPLETED) {
                    beginCooldown(reason);
                } else {
                    if (heaterActuator) heaterActuator->setPower(0.0f);
                    if (fanActuator) fanActuator->setPower(0.0f);
                }
                continue;
            }

            float targetTemp = session.target_temp_c;
            float controlOutput = controlEngine.compute(targetTemp, chamberTemp, dt);
            if (isnan(controlOutput)) {
                controlOutput = 0.0f;
            }
            controlOutput = constrain(controlOutput, 0.0f, 100.0f);

            if (heaterActuator) {
                heaterActuator->setPower(controlOutput);
            }
            if (fanActuator) {
                fanActuator->setPower((controlOutput > 0 || chamberTemp > 40.0f) ? 80.0f : 0.0f);
            }
        }

        float heaterPct = heaterActuator ? heaterActuator->getState().power_pct : 0.0f;
        bool heaterOn = heaterActuator ? heaterActuator->getState().enabled : false;
        float fanPct = fanActuator ? fanActuator->getState().power_pct : 0.0f;
        bool fanOn = fanActuator ? fanActuator->getState().enabled : false;

        stateMachine.updateDryingProgress(chamberTemp, chamberHumidity,
                                          heaterPct, heaterOn, fanPct, fanOn);

        static int logCounter = 0;
        if (++logCounter >= 50) {
            logCounter = 0;

            if (stateMachine.isDrying()) {
                logMgr.logDryingTelemetry(chamberTemp,
                    stateMachine.getCurrentSession().target_temp_c,
                    chamberHumidity,
                    stateMachine.getCurrentSession().target_humidity_pct,
                    heaterPct, fanPct,
                    stateMachine.getCurrentSession().elapsed_sec,
                    stateMachine.getCurrentSession().remaining_sec);
            }

            JsonDocument telemetry;
            telemetry["status"] = stateMachine.getStateName();
            telemetry["chamber_temp_c"] = chamberTemp;
            telemetry["target_temp_c"] = stateMachine.getCurrentSession().target_temp_c;
            telemetry["humidity_pct"] = chamberHumidity;
            telemetry["target_humidity_pct"] = stateMachine.getCurrentSession().target_humidity_pct;
            telemetry["heater_on"] = heaterOn;
            telemetry["heater_power_pct"] = heaterPct;
            telemetry["exhaust_fan_on"] = fanOn;
            telemetry["exhaust_fan_power_pct"] = fanPct;
            telemetry["elapsed_time_sec"] = stateMachine.getCurrentSession().elapsed_sec;
            telemetry["remaining_time_sec"] = stateMachine.getCurrentSession().remaining_sec;
            telemetry["cpu_usage_pct"] = sysMetrics.getCpuUsagePercent();
            telemetry["memory_free_bytes"] = sysMetrics.getFreeHeapBytes();
            telemetry["uptime_sec"] = millis() / 1000;
            telemetry["control_algorithm"] = controlEngine.getCurrentAlgorithmType();

            wsServer.broadcastTelemetry(telemetry.as<JsonObject>());
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
        wsServer.loop();
        
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
    const TickType_t period = pdMS_TO_TICKS(200);
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWakeTime, period);

        DisplayConfig dispConfig = configMgr.getDisplayConfig();
        if (!dispConfig.enabled) continue;
        if (!displayManager.isRefreshDue()) continue;

        JsonDocument statusDoc;
        JsonObject statusFields = statusDoc.to<JsonObject>();
        DryingSession session = stateMachine.getCurrentSession();

        statusFields["chamber_temp_c"] = session.target_temp_c;
        statusFields["target_temp_c"] = session.target_temp_c;
        statusFields["humidity_pct"] = session.target_humidity_pct;
        statusFields["heater_power_pct"] = heaterActuator ? heaterActuator->getState().power_pct : 0;
        statusFields["status"] = stateMachine.getStateName();

        displayManager.update(statusFields);
        displayManager.markRefreshed();
    }
}

// ============================================================
// Callbacks
// ============================================================
void onStateChange(SystemState oldState, SystemState newState) {
    String oldStr = stateMachine.getStateName();
    String newStr;
    switch (newState) {
        case SystemState::BOOT: newStr = "BOOT"; break;
        case SystemState::WIFI_CONNECT: newStr = "WIFI_CONNECT"; break;
        case SystemState::HOTSPOT: newStr = "HOTSPOT"; break;
        case SystemState::READY: newStr = "READY"; break;
        case SystemState::DRYING: newStr = "DRYING"; break;
        case SystemState::STOPPED: newStr = "STOPPED"; break;
        case SystemState::FAULT_STOPPED: newStr = "FAULT_STOPPED"; break;
    }
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, 
                     "State transition: %s -> %s", oldStr.c_str(), newStr.c_str());
    
    // On fault, trigger emergency stop on actuators
    if (newState == SystemState::FAULT_STOPPED) {
        if (heaterActuator) heaterActuator->emergencyStop();
        if (fanActuator) fanActuator->emergencyStop();
    }
}

void onSessionUpdate(const DryingSession& session) {
    // Update display, log, etc.
}

void onSafetyFault(FaultCode fault, const String& message) {
    logMgr.logSystem(LogLevel::ERROR, LogModule::SAFETY, "%s", message.c_str());
    wsServer.broadcastFault(fault, message);
    pluginMgr.callOnFault(fault, message);
    
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
    
    wsServer.broadcastPidCalibrate(doc.as<JsonObject>());
}

void onPidCalibrateComplete(const PidAutotuneController::Result& result) {
    if (result.success) {
        // Save to NVS
        PidConfig cfg(result.kp, result.ki, result.kd);
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
    wsServer.broadcastPidCalibrate(doc.as<JsonObject>());
}

void onLogCallback(const String& line, bool isDryingLog) {
    wsServer.broadcastLog(line, isDryingLog);
}

// ============================================================
// Setup & Initialization
// ============================================================
void initializeSensors() {
    SensorConfig sensorCfg = configMgr.getSensorConfig();
    JsonDocument sensorDoc;
    JsonObject sensorConfig = sensorDoc.to<JsonObject>();
    sensorConfig["type"] = sensorCfg.type;
    sensorConfig["is_integrated"] = sensorCfg.is_integrated;
    sensorConfig["i2c_bus"] = sensorCfg.i2c_bus;
    sensorConfig["i2c_address"] = sensorCfg.i2c_address;
    sensorConfig["gpio_pin"] = sensorCfg.gpio_pin;
    sensorConfig["sda_pin"] = sensorCfg.sda_pin;
    sensorConfig["scl_pin"] = sensorCfg.scl_pin;

    String type = sensorConfig["type"] | "sht31";
    ISensorDriver* sensor = DriverRegistry::instance().createSensor(type, sensorConfig);
    if (!sensor && type != "sht3x") {
        sensor = DriverRegistry::instance().createSensor("sht3x", sensorConfig);
    }

    activeTempSensor = sensor;
    activeHumiditySensor = sensor;

    if (!activeTempSensor) {
        logMgr.logSystem(LogLevel::WARNING, LogModule::SENSOR,
                         "No temperature sensor detected!");
    } else {
        logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR,
                         "Sensor %s initialized", type.c_str());
    }
}

void initializeActuators() {
    ActuatorConfig actuatorCfg = configMgr.getActuatorConfig();
    JsonDocument heaterDoc;
    JsonObject heaterConfig = heaterDoc.to<JsonObject>();
    heaterConfig["heater_pin"] = actuatorCfg.heater_pin;
    heaterConfig["gpio_pin"] = actuatorCfg.heater_pin;
    heaterConfig["pwm"] = actuatorCfg.heater_pin;
    heaterConfig["heater_pwm_freq"] = actuatorCfg.heater_pwm_freq;
    heaterConfig["heater_max_power_pct"] = actuatorCfg.heater_max_power_pct;

    JsonDocument fanDoc;
    JsonObject fanConfig = fanDoc.to<JsonObject>();
    fanConfig["fan_pin"] = actuatorCfg.fan_pin;
    fanConfig["gpio_pin"] = actuatorCfg.fan_pin;
    fanConfig["pwm"] = actuatorCfg.fan_pin;
    fanConfig["fan_mode"] = actuatorCfg.fan_mode;
    fanConfig["fan_pwm_freq"] = actuatorCfg.fan_pwm_freq;
    fanConfig["cooldown_duration_sec"] = actuatorCfg.cooldown_duration_sec;

    heaterActuator = DriverRegistry::instance().createActuator("mosfet_pwm", heaterConfig);
    fanActuator = DriverRegistry::instance().createActuator("fan_pwm", fanConfig);

    if (heaterActuator) {
        logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR,
                         "Heater actuator on GPIO %d", actuatorCfg.heater_pin);
    }
    if (fanActuator) {
        logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR,
                         "Fan actuator on GPIO %d", actuatorCfg.fan_pin);
    }
}

void initializeDisplays() {
    DisplayConfig displayCfg = configMgr.getDisplayConfig();
    if (!displayCfg.enabled) return;

    JsonDocument displayDoc;
    JsonObject dispConfig = displayDoc.to<JsonObject>();
    dispConfig["enabled"] = displayCfg.enabled;
    dispConfig["driver"] = displayCfg.driver;
    dispConfig["bus_type"] = displayCfg.bus_type;
    dispConfig["width"] = displayCfg.width;
    dispConfig["height"] = displayCfg.height;
    dispConfig["rotation"] = displayCfg.rotation;
    dispConfig["spi_mosi"] = displayCfg.spi_mosi;
    dispConfig["spi_sclk"] = displayCfg.spi_sclk;
    dispConfig["spi_cs"] = displayCfg.spi_cs;
    dispConfig["dc_pin"] = displayCfg.dc_pin;
    dispConfig["rst_pin"] = displayCfg.rst_pin;
    dispConfig["backlight_pin"] = displayCfg.backlight_pin;
    JsonArray fieldArray = dispConfig["fields"].to<JsonArray>();
    for (const String& field : displayCfg.fields) {
        fieldArray.add(field);
    }

    if (displayManager.begin(dispConfig)) {
        activeDisplay = nullptr;
        logMgr.logSystem(LogLevel::INFO, LogModule::DISPLAY_MODULE,
                         "Display %s initialized", displayManager.getActiveType().c_str());
    }
}

void initializeControlAlgorithm() {
    controlEngine.begin();
    JsonObject controlConfig = configMgr.getObjectConfig("control");
    String algorithm = "pid";
    JsonDocument fallback;
    JsonObject params = fallback.to<JsonObject>();

    if (!controlConfig.isNull()) {
        algorithm = controlConfig["algorithm"] | "pid";
        if (controlConfig["parameters"].is<JsonObject>()) {
            params = controlConfig["parameters"].as<JsonObject>();
        }
    } else {
        PidConfig pidCfg = configMgr.getPidConfig();
        params["kp"] = pidCfg.kp;
        params["ki"] = pidCfg.ki;
        params["kd"] = pidCfg.kd;
    }

    if (!controlEngine.setAlgorithm(algorithm, params)) {
        JsonDocument bang;
        JsonObject bangParams = bang.to<JsonObject>();
        bangParams["hysteresis_c"] = 1.0f;
        controlEngine.setAlgorithm("bang_bang", bangParams);
    }
}

void reloadHardwareFromConfig() {
    initializeSensors();
    initializeActuators();
    initializeDisplays();
    initializeControlAlgorithm();
}

void initializeNetwork() {
    wifiMgr.begin();

    wifiMgr.setStatusCallback([](WifiManager::Status s) {
        if (s == WifiManager::Status::CONNECTED) {
            logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK,
                             "WiFi connected: %s", wifiMgr.getLocalIP().c_str());
            stateMachine.transitionTo(SystemState::READY);
        } else if (s == WifiManager::Status::AP_ACTIVE) {
            stateMachine.transitionTo(SystemState::HOTSPOT);
        }
    });

    if (webServer.begin(&configMgr, &logMgr, &hwParser, &DriverRegistry::instance(), &wifiMgr)) {
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK,
                         "HTTP server started on port 80");
    }

    if (wsServer.begin(&configMgr, &stateMachine, &safetyEngine, &logMgr, &pidAutotune,
                       &hwParser, &DriverRegistry::instance(), &profileMgr, &controlEngine)) {
        webServer.attachWebSocket(wsServer.getWebSocket());
        wsServer.setHardwareReloadCallback(reloadHardwareFromConfig);
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK,
                         "WebSocket server started on ws://<ip>/ws");
    }
}

void setup() {
    // Initialize LittleFS
    if (!LittleFS.begin()) {
    }
    
    // Initialize ConfigManager (loads all NVS configs)
    if (!configMgr.begin()) {
    }
    
    // Initialize core subsystems
    if (!stateMachine.begin()) {
    }
    
    if (!logMgr.begin()) {
    }
    
    SafetyConfig safetyCfg;
    safetyCfg.hard_temp_limit_c = 80.0f;
    safetyCfg.watchdog_enabled = true;
    if (!safetyEngine.begin(safetyCfg)) {
    }
    
    // Setup callbacks
    stateMachine.setStateChangeCallback(onStateChange);
    stateMachine.setSessionUpdateCallback(onSessionUpdate);
    safetyEngine.setFaultCallback(onSafetyFault);
    pidAutotune.setHeaterCallback([](float power) { 
        // This will be called by PID autotune during calibration
    });
    logMgr.setLogCallback(onLogCallback);
    
    pidAutotune.setHeaterCallback([](float power) {
        if (heaterActuator) {
            heaterActuator->setPower(power);
        }
    });
    logMgr.setLogCallback(onLogCallback);

    DriverRegistry::instance().registerBuiltins();

    pluginMgr.begin();
    pluginMgr.callOnInit(configMgr);
    pluginMgr.callOnStart();

    initializeSensors();
    initializeActuators();
    initializeDisplays();
    initializeControlAlgorithm();

    sysMetrics.begin();
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
    
}

void loop() {
    // Main loop is empty - all work done in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}