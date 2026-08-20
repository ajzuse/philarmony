/*
 * Philarmony Filament Dryer ESP32 Firmware
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
#include "timing_contracts.h"
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
#include "drivers/touch/TouchManager.hpp"
#include "ui/TouchUiController.hpp"
#include "ui/UiApp.hpp"
#include "ui/history/CycleHistoryStore.hpp"
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
TouchManager touchManager;
TouchUiController touchUiController(stateMachine, profileMgr, configMgr,
                                    &safetyEngine);
CycleHistoryStore cycleHistory;
UiApp* uiApp = nullptr;
PluginManager pluginMgr;
WifiManager wifiMgr;
WebServer webServer(80);
WebSocketServer wsServer;
SystemMetrics sysMetrics;

ISensorDriver* activeTempSensor = nullptr;
ISensorDriver* activeHumiditySensor = nullptr;
ISensorDriver* extraTempSensor = nullptr;
IActuatorDriver* heaterActuator = nullptr;
IActuatorDriver* fanActuator = nullptr;
IActuatorDriver* customActuator = nullptr;
IDisplayDriver* activeDisplay = nullptr;
static bool actuatorsShareInstance = false;
static float lastCommandedHeaterPct = 0.0f;

static bool cooldownActive = false;
static uint32_t cooldownEndMs = 0;
static DryingStopReason pendingCooldownReason = DryingStopReason::COMPLETED;

// Shared live readings for display/telemetry (written by control loop)
static volatile float g_liveChamberTemp = NAN;
static volatile float g_liveChamberHumidity = NAN;
static volatile float g_liveHeaterPowerPct = 0.0f;
static volatile float g_liveFanPowerPct = 0.0f;
static volatile bool g_touchUiOwnsDisplay = false;

static void cutActuatorPower() {
    if (heaterActuator) {
        heaterActuator->setPower(0.0f);
        heaterActuator->emergencyStop();
    }
    if (fanActuator) {
        fanActuator->setPower(0.0f);
        fanActuator->emergencyStop();
    }
}

static void teardownDrivers() {
    cutActuatorPower();
    displayManager.end();
    if (activeTempSensor != nullptr && activeTempSensor == activeHumiditySensor) {
        delete activeTempSensor;
        activeTempSensor = nullptr;
        activeHumiditySensor = nullptr;
    } else {
        delete activeTempSensor;
        activeTempSensor = nullptr;
        delete activeHumiditySensor;
        activeHumiditySensor = nullptr;
    }
    delete extraTempSensor;
    extraTempSensor = nullptr;

    if (actuatorsShareInstance) {
        delete heaterActuator;
        heaterActuator = nullptr;
        fanActuator = nullptr;
        actuatorsShareInstance = false;
    } else {
        delete heaterActuator;
        heaterActuator = nullptr;
        delete fanActuator;
        fanActuator = nullptr;
    }
    delete customActuator;
    customActuator = nullptr;
}

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

static float resolveFanDutyPct(float chamber_temp_c) {
    ActuatorConfig ac = configMgr.getActuatorConfig();
    if (!ac.fan_speed_curve.empty()) {
        const auto& curve = ac.fan_speed_curve;
        bool has_temps = false;
        for (const auto& pt : curve) {
            if (pt.temp_c > 0.0f) {
                has_temps = true;
                break;
            }
        }
        if (!has_temps || isnan(chamber_temp_c)) {
            return constrain(curve.front().power_pct, 0.0f, 100.0f);
        }
        if (chamber_temp_c <= curve.front().temp_c) {
            return constrain(curve.front().power_pct, 0.0f, 100.0f);
        }
        if (chamber_temp_c >= curve.back().temp_c) {
            return constrain(curve.back().power_pct, 0.0f, 100.0f);
        }
        for (size_t i = 1; i < curve.size(); ++i) {
            if (chamber_temp_c <= curve[i].temp_c) {
                const float t0 = curve[i - 1].temp_c;
                const float t1 = curve[i].temp_c;
                const float p0 = curve[i - 1].power_pct;
                const float p1 = curve[i].power_pct;
                if (t1 <= t0) return constrain(p1, 0.0f, 100.0f);
                const float frac = (chamber_temp_c - t0) / (t1 - t0);
                return constrain(p0 + frac * (p1 - p0), 0.0f, 100.0f);
            }
        }
        return constrain(curve.back().power_pct, 0.0f, 100.0f);
    }
    return constrain(ac.fan_duty_pct, 0.0f, 100.0f);
}

static void beginCooldown(DryingStopReason reason) {
    if (!stateMachine.beginCooldown(reason)) {
        stateMachine.stopDrying(reason);
        cutActuatorPower();
        return;
    }
    pendingCooldownReason = reason;
    cooldownActive = true;
    ActuatorConfig actuatorCfg = configMgr.getActuatorConfig();
    cooldownEndMs = millis() + (actuatorCfg.cooldown_duration_sec * 1000UL);
    if (heaterActuator) {
        heaterActuator->setPower(0.0f);
    }
    if (fanActuator) {
        fanActuator->setPower(resolveFanDutyPct(g_liveChamberTemp));
    }
}

// ============================================================
// FreeRTOS Task Handles
// ============================================================
TaskHandle_t controlLoopTaskHandle = nullptr;
TaskHandle_t networkTaskHandle = nullptr;
TaskHandle_t displayTaskHandle = nullptr;
TaskHandle_t uiTaskHandle = nullptr;

// ============================================================
// Control Loop (Core 1) - 50Hz = 20ms period
// ============================================================
void controlLoopTask(void* pvParameters) {
    const TickType_t period = pdMS_TO_TICKS(kControlLoopPeriodMs);
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    // PID timing
    uint32_t lastPidTime = millis();
    static uint32_t lastBusCheckMs = 0;

    safetyEngine.attachCurrentTaskToWatchdog();
    
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
        g_liveChamberTemp = chamberTemp;
        g_liveChamberHumidity = chamberHumidity;

        // Periodic I2C bus recovery when sensor reads fail
        if (now - lastBusCheckMs >= 1000) {
            lastBusCheckMs = now;
            if (!tempReading.valid && activeTempSensor) {
                safetyEngine.detectAndRecoverI2CBusLockup();
            }
            static bool displayWasOk = false;
            const bool displayOk = !configMgr.getDisplayConfig().enabled ||
                                   displayManager.isAnyConnected();
            if (displayWasOk && !displayOk) {
                safetyEngine.detectAndRecoverSpiBusError(true);
            }
            displayWasOk = displayOk;
        }

        if (cooldownActive) {
            if (heaterActuator) heaterActuator->setPower(0.0f);
            if (fanActuator) fanActuator->setPower(resolveFanDutyPct(chamberTemp));
            const bool cooldownDone = millis() >= cooldownEndMs;
            if (cooldownDone) {
                cooldownActive = false;
                if (fanActuator) fanActuator->setPower(0.0f);
                stateMachine.completeCooldown();
                pluginMgr.callOnSessionStop(stateMachine.getCurrentSession(), pendingCooldownReason);
            }

            // T121: 1Hz status/update during COOLDOWN (heater 0%, fan on)
            static int cooldownLogCounter = 0;
            if (++cooldownLogCounter >= kStatusLoopsPerBroadcast) {
                cooldownLogCounter = 0;
                const float heaterPct = 0.0f;
                const float fanPct = fanActuator ? fanActuator->getState().power_pct : 0.0f;
                const bool fanOn = fanPct > 0.5f;
                g_liveHeaterPowerPct = heaterPct;
                g_liveFanPowerPct = fanPct;
                stateMachine.updateDryingProgress(chamberTemp, chamberHumidity,
                                                  heaterPct, false, fanPct, fanOn);
                JsonDocument telemetry;
                JsonObject payload = telemetry.to<JsonObject>();
                wsServer.buildStatusPayload(payload);
                payload["chamber_temp_c"] = chamberTemp;
                payload["humidity_pct"] = chamberHumidity;
                payload["heater_on"] = false;
                payload["heater_power_pct"] = 0.0f;
                payload["exhaust_fan_on"] = fanOn;
                payload["exhaust_fan_power_pct"] = fanPct;
                payload["cpu_usage_pct"] = sysMetrics.getCpuUsagePercent();
                payload["memory_free_bytes"] = sysMetrics.getFreeHeapBytes();
                payload["uptime_sec"] = millis() / 1000;
                wsServer.broadcastTelemetry(payload);
            }
            continue;
        }
        
        float heaterPowerPct = (heaterActuator && heaterActuator->getState().enabled)
                                   ? heaterActuator->getState().power_pct
                                   : 0.0f;
        uint8_t heaterPower = static_cast<uint8_t>(heaterPowerPct);
        g_liveHeaterPowerPct = heaterPowerPct;
        g_liveFanPowerPct = fanActuator ? fanActuator->getState().power_pct : 0.0f;

        if (heaterActuator) {
            bool overcurrent = false;
            bool measured_from_feedback = false;
            float measured = heaterActuator->getState().power_pct;
            if (heaterActuator->hasFeedback()) {
                overcurrent = heaterActuator->checkOvercurrent();
                measured = heaterActuator->getMeasuredPowerPct();
                measured_from_feedback = true;
            }
            // Never pass !isHealthy() as overcurrent — e-stop must not count (T118)
            safetyEngine.checkActuatorFault(lastCommandedHeaterPct, measured,
                                            overcurrent, measured_from_feedback);
        }

        bool sensorConnected = tempReading.valid;
        if (!safetyEngine.checkSafety(chamberTemp,
                                       stateMachine.getCurrentSession().target_temp_c,
                                       heaterPower, heaterPower > 0, sensorConnected)) {
            // SafetyEngine already waited sensor_timeout_ms before faulting disconnect
            stateMachine.stopDrying(DryingStopReason::SENSOR_ERROR);
            cutActuatorPower();
            continue;
        }

        if (pidAutotune.isRunning()) {
            pidAutotune.update(chamberTemp, heaterPower);
        }

        if (stateMachine.isPaused()) {
            cutActuatorPower();
            if (stateMachine.tickPauseTimeout()) {
                cutActuatorPower();
            }
            continue;
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

            if (!isnan(chamberTemp) &&
                chamberTemp >= safetyEngine.getConfig().hard_temp_limit_c) {
                stop = true;
                reason = DryingStopReason::SAFETY_CUTOFF;
            }

            // T115: do NOT abort on first invalid sample — SafetyEngine honors sensor_timeout_ms
            if (!sensorConnected) {
                // Surface error in status while session continues until timeout/fault
                session.stop_reason = DryingStopReason::SENSOR_ERROR;
            }

            if (stop) {
                if (reason == DryingStopReason::HUMIDITY_REACHED ||
                    reason == DryingStopReason::MAX_TIME ||
                    reason == DryingStopReason::COMPLETED) {
                    beginCooldown(reason);
                } else {
                    stateMachine.stopDrying(reason);
                    cutActuatorPower();
                }
                continue;
            }

            if (!pidAutotune.isRunning()) {
                float targetTemp = session.target_temp_c;
                float controlOutput = controlEngine.compute(targetTemp, chamberTemp, dt);
                if (isnan(controlOutput)) {
                    controlOutput = 0.0f;
                }
                // Limit heater while sensor invalid (fail-safe soft limit)
                if (!sensorConnected) {
                    controlOutput = 0.0f;
                }
                controlOutput = constrain(controlOutput, 0.0f, 100.0f);
                const uint8_t maxPwr = safetyEngine.getConfig().max_heater_power_pct;
                if (controlOutput > maxPwr) {
                    controlOutput = static_cast<float>(maxPwr);
                }

                if (heaterActuator) {
                    heaterActuator->setPower(controlOutput);
                    lastCommandedHeaterPct = controlOutput;
                }
                if (fanActuator && fanActuator != heaterActuator) {
                    const float fanDuty = resolveFanDutyPct(chamberTemp);
                    fanActuator->setPower(
                        (controlOutput > 0 || chamberTemp > 40.0f) ? fanDuty : 0.0f);
                }
            }
        }

        float heaterPct = heaterActuator ? heaterActuator->getState().power_pct : 0.0f;
        bool heaterOn = heaterActuator ? heaterActuator->getState().enabled : false;
        float fanPct = fanActuator ? fanActuator->getState().power_pct : 0.0f;
        bool fanOn = fanActuator ? fanActuator->getState().enabled : false;
        g_liveHeaterPowerPct = heaterPct;
        g_liveFanPowerPct = fanPct;

        stateMachine.updateDryingProgress(chamberTemp, chamberHumidity,
                                          heaterPct, heaterOn, fanPct, fanOn);

        static int logCounter = 0;
        if (++logCounter >= kStatusLoopsPerBroadcast) {
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
            JsonObject payload = telemetry.to<JsonObject>();
            wsServer.buildStatusPayload(payload);
            payload["chamber_temp_c"] = chamberTemp;
            payload["humidity_pct"] = chamberHumidity;
            payload["heater_on"] = heaterOn;
            payload["heater_power_pct"] = heaterPct;
            payload["exhaust_fan_on"] = fanOn;
            payload["exhaust_fan_power_pct"] = fanPct;
            payload["cpu_usage_pct"] = sysMetrics.getCpuUsagePercent();
            payload["memory_free_bytes"] = sysMetrics.getFreeHeapBytes();
            payload["uptime_sec"] = millis() / 1000;
            payload["control_algorithm"] = controlEngine.getCurrentAlgorithmType();
            payload["sensor_type"] = configMgr.getSensorConfig().type;
            if (!configMgr.getSensorConfig().humidity_type.isEmpty()) {
                payload["humidity_sensor_type"] = configMgr.getSensorConfig().humidity_type;
            }
            if (extraTempSensor) {
                SensorReading extra = applySensorCalibration(extraTempSensor->read());
                if (extra.valid) {
                    payload["external_temp_c"] = extra.temperature;
                }
                payload["extra_temp_type"] = configMgr.getSensorConfig().extra_temp_type;
            }

            StatusPayload pluginTelemetry;
            pluginTelemetry.status = stateMachine.getStatusStreamName();
            pluginTelemetry.chamber_temp_c = chamberTemp;
            pluginTelemetry.humidity_pct = chamberHumidity;
            pluginTelemetry.heater_power_pct = heaterPct;
            pluginMgr.callOnTelemetryTick(pluginTelemetry);

            wsServer.broadcastTelemetry(payload);
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

        // CPU/heap metrics (throttled internally to sample_interval)
        sysMetrics.update();
        
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
void uiTask(void* pvParameters) {
    (void)pvParameters;
    const TickType_t period = pdMS_TO_TICKS(33); // ~30fps
    TickType_t lastWakeTime = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&lastWakeTime, period);
        if (uiApp) {
            uiApp->setLiveReadings(g_liveChamberTemp, g_liveChamberHumidity);
            uiApp->loop();
        }
    }
}

void displayTask(void* pvParameters) {
    const TickType_t period = pdMS_TO_TICKS(200);
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWakeTime, period);

        DisplayConfig dispConfig = configMgr.getDisplayConfig();
        if (!dispConfig.enabled) continue;
        if (g_touchUiOwnsDisplay) continue;
        if (!displayManager.isRefreshDue()) continue;

        JsonDocument statusDoc;
        JsonObject statusFields = statusDoc.to<JsonObject>();
        DryingSession session = stateMachine.getCurrentSession();

        statusFields["chamber_temp_c"] = g_liveChamberTemp;
        statusFields["target_temp_c"] = session.target_temp_c;
        statusFields["humidity_pct"] = g_liveChamberHumidity;
        statusFields["target_humidity_pct"] = session.target_humidity_pct;
        statusFields["heater_power_pct"] = g_liveHeaterPowerPct;
        statusFields["exhaust_fan_power_pct"] = g_liveFanPowerPct;
        statusFields["heater_on"] = g_liveHeaterPowerPct > 0.5f;
        statusFields["exhaust_fan_on"] = g_liveFanPowerPct > 0.5f;
        statusFields["elapsed_time_sec"] = session.elapsed_sec;
        statusFields["remaining_time_sec"] = session.remaining_sec;
        statusFields["cpu_usage_pct"] = sysMetrics.getCpuUsagePercent();
        statusFields["memory_free_bytes"] = sysMetrics.getFreeHeapBytes();
        statusFields["uptime_sec"] = millis() / 1000;
        statusFields["status"] = stateMachine.getStatusStreamName();
        statusFields["font_scaling"] = static_cast<int>(displayManager.getLayout().font_scaling);
        statusFields["compact_mode"] = displayManager.getLayout().compact_mode;

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
        case SystemState::COOLDOWN: newStr = "COOLDOWN"; break;
        case SystemState::STOPPED: newStr = "STOPPED"; break;
        case SystemState::FAULT_STOPPED: newStr = "FAULT_STOPPED"; break;
        case SystemState::PAUSED: newStr = "PAUSED"; break;
    }
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, 
                     "State transition: %s -> %s", oldStr.c_str(), newStr.c_str());
    
    // On fault or stop, clear cooldown gate and always cut actuators
    if (newState == SystemState::FAULT_STOPPED || newState == SystemState::STOPPED) {
        cooldownActive = false;
        cutActuatorPower();
        if (newState == SystemState::STOPPED) {
            cycleHistory.appendFromSession(stateMachine.getCurrentSession());
        }
        if (uiApp && wsServer.getUiSource() == "websocket") {
            uiApp->notifyRemoteStateChange();
        }
    }
    if (newState == SystemState::DRYING || newState == SystemState::PAUSED) {
        if (newState == SystemState::DRYING && oldState != SystemState::PAUSED) {
            cycleHistory.resetSampler();
        }
        if (uiApp && wsServer.getUiSource() == "websocket") {
            uiApp->notifyRemoteStateChange();
        }
        DryingSession interrupted;
        if (stateMachine.captureInterruptedSession(interrupted)) {
            configMgr.saveInterruptedSession(interrupted, newState);
        }
    } else if (newState == SystemState::STOPPED ||
               newState == SystemState::FAULT_STOPPED ||
               (newState == SystemState::READY &&
                (oldState == SystemState::STOPPED ||
                 oldState == SystemState::FAULT_STOPPED))) {
        configMgr.clearInterruptedSession();
    }
    if (newState == SystemState::PAUSED) {
        cutActuatorPower();
    }
    if (newState == SystemState::DRYING) {
        pluginMgr.callOnSessionStart(stateMachine.getCurrentSession());
    }
}

void onSessionUpdate(const DryingSession& session) {
    static uint32_t last_persist_ms = 0;
    static uint32_t last_sample_ms = 0;
    if (stateMachine.isDrying() && millis() - last_sample_ms >= 1000) {
        last_sample_ms = millis();
        cycleHistory.recordSample(session.current_temp_c,
                                  session.current_humidity_pct);
    }
    if ((stateMachine.isDrying() || stateMachine.isPaused()) &&
        millis() - last_persist_ms >= 5000) {
        last_persist_ms = millis();
        configMgr.saveInterruptedSession(session, stateMachine.getState());
    }
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
    // saved_to_nvs only after successful NVS write in onPidCalibrateComplete
    doc["saved_to_nvs"] = false;
    
    wsServer.broadcastPidCalibrate(doc.as<JsonObject>());
}

void onPidCalibrateComplete(const PidAutotuneController::Result& result) {
    bool saved = false;
    if (result.success) {
        PidConfig cfg(result.kp, result.ki, result.kd);
        saved = configMgr.setPidConfig(cfg);

        JsonDocument paramsDoc;
        JsonObject params = paramsDoc.to<JsonObject>();
        params["kp"] = result.kp;
        params["ki"] = result.ki;
        params["kd"] = result.kd;
        if (result.hysteresis > 0.0f) {
            params["hysteresis"] = result.hysteresis;
        }
        if (result.base_pwm > 0.0f) {
            params["base_pwm"] = result.base_pwm;
        }
        if (result.temp_coefficient != 0.0f) {
            params["temp_coefficient"] = result.temp_coefficient;
        }

        // T127/T161: apply tuned params to current algorithm when it accepts them
        IControlAlgorithm* current = controlEngine.getCurrentAlgorithm();
        const String currentType = controlEngine.getCurrentAlgorithmType();
        const String tunedAlgo = result.algorithm.length() ? result.algorithm : String("pid");
        if (current && (currentType == tunedAlgo || current->needsTuning())) {
            current->setParameters(params);
        } else if (currentType.isEmpty() || currentType == tunedAlgo) {
            controlEngine.setAlgorithm(tunedAlgo.c_str(), params);
        }
        
        logMgr.logSystem(LogLevel::INFO, LogModule::PID, 
                         "Auto-tune complete (%s): Kp=%.2f Ki=%.2f Kd=%.2f hyst=%.2f nvs=%d",
                         tunedAlgo.c_str(), result.kp, result.ki, result.kd,
                         result.hysteresis, saved ? 1 : 0);
    } else {
        logMgr.logSystem(LogLevel::ERROR, LogModule::PID, 
                         "PID auto-tune failed: %s", result.error.c_str());
    }
    
    StaticJsonDocument<256> doc;
    doc["status"] = result.success ? "complete" : "error";
    doc["saved_to_nvs"] = saved;
    if (result.success) {
        doc["kp"] = result.kp;
        doc["ki"] = result.ki;
        doc["kd"] = result.kd;
        if (result.hysteresis > 0.0f) doc["hysteresis"] = result.hysteresis;
        if (result.base_pwm > 0.0f) doc["base_pwm"] = result.base_pwm;
        if (result.temp_coefficient != 0.0f) doc["temp_coefficient"] = result.temp_coefficient;
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
void applySafetyConfigFromManager() {
    SafetyConfig cfg = safetyEngine.getConfig();
    JsonObject control = configMgr.getObjectConfig("control");
    if (!control.isNull() && control["safety_limits"].is<JsonObject>()) {
        JsonObject safety = control["safety_limits"].as<JsonObject>();
        cfg.hard_temp_limit_c = safety["hard_temp_limit_c"] | cfg.hard_temp_limit_c;
        cfg.max_heater_power_pct = safety["max_heater_power_pct"] | cfg.max_heater_power_pct;
        cfg.sensor_timeout_ms = safety["sensor_timeout_ms"] | cfg.sensor_timeout_ms;
        cfg.thermal_runaway_time_sec =
            safety["thermal_runaway_time_sec"] | cfg.thermal_runaway_time_sec;
        cfg.thermal_runaway_temp_rise =
            safety["thermal_runaway_temp_rise_c"] | cfg.thermal_runaway_temp_rise;
    }
    SensorConfig sc = configMgr.getSensorConfig();
    cfg.i2c_sda_pin = sc.sda_pin;
    cfg.i2c_scl_pin = sc.scl_pin;
    ActuatorConfig ac = configMgr.getActuatorConfig();
    cfg.max_heater_power_pct = ac.heater_max_power_pct;
    cfg.cooldown_fan_duration_sec = ac.cooldown_duration_sec;
    if (ac.heater_max_temp_c > 0.0f) {
        cfg.hard_temp_limit_c = min(cfg.hard_temp_limit_c, ac.heater_max_temp_c);
    }
    safetyEngine.setConfig(cfg);
}

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
    if (!sensor) {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR,
                         "Failed to create sensor type '%s' — no fallback", type.c_str());
    }

    activeTempSensor = sensor;
    if (sensorCfg.is_integrated || sensorCfg.humidity_type.isEmpty()) {
        activeHumiditySensor = sensor;
    } else {
        JsonDocument humDoc;
        JsonObject humConfig = humDoc.to<JsonObject>();
        humConfig["type"] = sensorCfg.humidity_type;
        humConfig["i2c_address"] = sensorCfg.humidity_i2c_address;
        humConfig["gpio_pin"] = sensorCfg.humidity_gpio_pin;
        humConfig["sda_pin"] = sensorCfg.humidity_sda_pin;
        humConfig["scl_pin"] = sensorCfg.humidity_scl_pin;
        activeHumiditySensor =
            DriverRegistry::instance().createSensor(sensorCfg.humidity_type, humConfig);
        if (!activeHumiditySensor) {
            logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR,
                             "Failed to create humidity sensor '%s'",
                             sensorCfg.humidity_type.c_str());
        }
    }

    extraTempSensor = nullptr;
    if (!sensorCfg.extra_temp_type.isEmpty()) {
        JsonDocument extraDoc;
        JsonObject extraConfig = extraDoc.to<JsonObject>();
        extraConfig["type"] = sensorCfg.extra_temp_type;
        extraConfig["gpio_pin"] = sensorCfg.extra_temp_gpio_pin;
        extraConfig["i2c_address"] = sensorCfg.extra_temp_i2c_address;
        extraConfig["sda_pin"] = sensorCfg.sda_pin;
        extraConfig["scl_pin"] = sensorCfg.scl_pin;
        extraTempSensor =
            DriverRegistry::instance().createSensor(sensorCfg.extra_temp_type, extraConfig);
        if (!extraTempSensor) {
            logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR,
                             "Failed to create extra temp sensor '%s'",
                             sensorCfg.extra_temp_type.c_str());
        }
    }

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
    fanConfig["heater_pin"] = actuatorCfg.heater_pin;

    String heaterType = actuatorCfg.heater_type.length() ? actuatorCfg.heater_type : "mosfet_pwm";
    String fanType = actuatorCfg.fan_type.length() ? actuatorCfg.fan_type : "fan_pwm";
    if (fanType == "fan_digital") {
        fanConfig["type"] = "fan_digital";
        fanConfig["fan_mode"] = "independent_digital";
    } else {
        fanConfig["type"] = fanType;
    }
    heaterConfig["type"] = heaterType;

    actuatorsShareInstance = false;
    const bool useShared =
        heaterType == "shared_mosfet" || fanType == "shared_mosfet" ||
        actuatorCfg.fan_mode == "shared_mosfet";

    if (useShared) {
        IActuatorDriver* shared =
            DriverRegistry::instance().createActuator("shared_mosfet", heaterConfig);
        heaterActuator = shared;
        fanActuator = shared;
        actuatorsShareInstance = (shared != nullptr);
        heaterType = "shared_mosfet";
        fanType = "shared_mosfet";
        if (!shared) {
            logMgr.logSystem(LogLevel::ERROR, LogModule::ACTUATOR,
                             "Failed to create shared_mosfet — no fallback");
        }
    } else {
        heaterActuator = DriverRegistry::instance().createActuator(heaterType, heaterConfig);
        if (!heaterActuator) {
            logMgr.logSystem(LogLevel::ERROR, LogModule::ACTUATOR,
                             "Failed to create heater '%s' — no fallback", heaterType.c_str());
        }
        fanActuator = DriverRegistry::instance().createActuator(fanType, fanConfig);
        if (!fanActuator) {
            logMgr.logSystem(LogLevel::ERROR, LogModule::ACTUATOR,
                             "Failed to create fan '%s' — no fallback", fanType.c_str());
        }
    }

    customActuator = nullptr;
    if (actuatorCfg.has_custom && !actuatorCfg.custom_type.isEmpty()) {
        JsonDocument customDoc;
        JsonObject customConfig = customDoc.to<JsonObject>();
        customConfig["gpio_pin"] = actuatorCfg.custom_pin;
        customConfig["pwm"] = actuatorCfg.custom_pin;
        customActuator =
            DriverRegistry::instance().createActuator(actuatorCfg.custom_type, customConfig);
    }

    if (heaterActuator) {
        logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR,
                         "Heater %s on GPIO %d", heaterType.c_str(), actuatorCfg.heater_pin);
    }
    if (fanActuator) {
        logMgr.logSystem(LogLevel::INFO, LogModule::ACTUATOR,
                         "Fan %s on GPIO %d", fanType.c_str(), actuatorCfg.fan_pin);
    }
}

void initializeDisplays() {
    DisplayConfig displayCfg = configMgr.getDisplayConfig();
    if (!displayCfg.enabled) {
        g_touchUiOwnsDisplay = false;
        displayManager.end();
        return;
    }

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
    dispConfig["sda_pin"] = displayCfg.i2c_sda;
    dispConfig["scl_pin"] = displayCfg.i2c_scl;
    dispConfig["i2c_sda"] = displayCfg.i2c_sda;
    dispConfig["i2c_scl"] = displayCfg.i2c_scl;
    dispConfig["i2c_address"] = displayCfg.i2c_address;
    dispConfig["address"] = displayCfg.i2c_address;
    dispConfig["refresh_rate_hz"] = displayCfg.refresh_rate_hz;
    JsonArray fieldArray = dispConfig["fields"].to<JsonArray>();
    for (const String& field : displayCfg.fields) {
        fieldArray.add(field);
    }

    if (displayManager.begin(dispConfig)) {
        // Apply layout (fields + refresh) after driver init
        JsonDocument layoutDoc;
        JsonObject layout = layoutDoc.to<JsonObject>();
        JsonArray layoutFields = layout["fields"].to<JsonArray>();
        for (const String& field : displayCfg.fields) {
            layoutFields.add(field);
        }
        layout["refresh_rate_hz"] = displayCfg.refresh_rate_hz;
        layout["font_scaling"] = displayCfg.font_scaling;
        layout["compact_mode"] = displayCfg.compact_mode;
        displayManager.setLayout(layout);

        activeDisplay = displayManager.getActiveDriver();
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

    if (algorithm == "custom") {
        logMgr.logSystem(LogLevel::ERROR, LogModule::PID,
                         "custom algorithm requires registered factory; not falling back");
        return;
    }

    if (!controlEngine.setAlgorithm(algorithm, params)) {
        logMgr.logSystem(LogLevel::WARNING, LogModule::PID,
                         "Failed to set algorithm '%s'; falling back to bang_bang",
                         algorithm.c_str());
        JsonDocument bang;
        JsonObject bangParams = bang.to<JsonObject>();
        bangParams["hysteresis_c"] = 1.0f;
        controlEngine.setAlgorithm("bang_bang", bangParams);
    }
}

void reloadHardwareFromConfig() {
    // Belt-and-suspenders: API should reject reload during drying/cooldown;
    // if called anyway, e-stop before tearing down live drivers.
    if (stateMachine.isDrying() || stateMachine.isCoolingDown() || stateMachine.isPaused()) {
        cutActuatorPower();
        stateMachine.stopDrying(DryingStopReason::SAFETY_CUTOFF);
    }
    teardownDrivers();
    initializeSensors();
    initializeActuators();
    initializeDisplays();
    initializeControlAlgorithm();
    applySafetyConfigFromManager();
}

void initializeNetwork() {
    wifiMgr.setConfigManager(&configMgr);
    wifiMgr.begin();

    wifiMgr.setStatusCallback([](WifiManager::Status s) {
        if (s == WifiManager::Status::CONNECTED) {
            logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK,
                             "WiFi connected: %s", wifiMgr.getLocalIP().c_str());
            if (wifiMgr.isNtpStarted()) {
                logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK,
                                 "SNTP time sync started");
            }
            stateMachine.transitionTo(SystemState::READY);
        } else if (s == WifiManager::Status::AP_ACTIVE) {
            stateMachine.transitionTo(SystemState::HOTSPOT);
        }
    });

    if (webServer.begin(&configMgr, &logMgr, &hwParser, &DriverRegistry::instance(), &wifiMgr,
                        &stateMachine)) {
        webServer.setHardwareReloadCallback(reloadHardwareFromConfig);
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK,
                         "HTTP server started on port 80");
    }

    if (wsServer.begin(&configMgr, &stateMachine, &safetyEngine, &logMgr, &pidAutotune,
                       &hwParser, &DriverRegistry::instance(), &profileMgr, &controlEngine)) {
        webServer.attachWebSocket(wsServer.getWebSocket());
        wsServer.setHardwareReloadCallback(reloadHardwareFromConfig);
        wsServer.setActuatorCutoffCallback(cutActuatorPower);
        wsServer.setSafetyRefreshCallback(applySafetyConfigFromManager);
        wsServer.setPidCalibrateCallbacks(onPidCalibrateProgress, onPidCalibrateComplete);
        wsServer.setPluginManager(&pluginMgr);
        webServer.setPluginManager(&pluginMgr);
        logMgr.logSystem(LogLevel::INFO, LogModule::NETWORK,
                         "WebSocket server started on ws://<ip>/ws");
    }
}

void setup() {
    // Initialize LittleFS
    if (!LittleFS.begin()) {
    }
    cycleHistory.begin();
    
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
    {
        SensorConfig sc = configMgr.getSensorConfig();
        safetyCfg.i2c_sda_pin = sc.sda_pin;
        safetyCfg.i2c_scl_pin = sc.scl_pin;
    }
    if (!safetyEngine.begin(safetyCfg)) {
    }
    applySafetyConfigFromManager();
    
    // Setup callbacks
    stateMachine.setStateChangeCallback(onStateChange);
    stateMachine.setSessionUpdateCallback(onSessionUpdate);
    safetyEngine.setFaultCallback(onSafetyFault);
    safetyEngine.setEmergencyShutdownCallback(cutActuatorPower);
    safetyEngine.setSpiRecoveryCallback([]() -> bool {
        DisplayConfig dc = configMgr.getDisplayConfig();
        if (!dc.enabled) {
            return true;
        }
        displayManager.end();
        initializeDisplays();
        return displayManager.isAnyConnected() || dc.bus_type != "spi";
    });
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

    DryingSession interruptedSession;
    SystemState interruptedState = SystemState::READY;
    if (configMgr.loadInterruptedSession(interruptedSession, interruptedState)) {
        stateMachine.restoreInterruptedSession(interruptedSession,
                                               interruptedState);
    }

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

    // Touch UI task (Core 1, below control priority)
    {
        JsonDocument touchDoc;
        JsonObject touchCfg = touchDoc.to<JsonObject>();
        TouchConfig savedTouch = configMgr.getTouchConfig();
        DisplayConfig dc = configMgr.getDisplayConfig();
        touchCfg["controller_type"] = savedTouch.controller_type;
        touchCfg["i2c_address"] = savedTouch.i2c_address;
        touchCfg["spi_cs"] = savedTouch.spi_cs;
        touchCfg["irq_pin"] = savedTouch.irq_pin;
        touchCfg["spi_mosi"] = savedTouch.spi_mosi;
        touchCfg["spi_miso"] = savedTouch.spi_miso;
        touchCfg["spi_sclk"] = savedTouch.spi_sclk;
        touchCfg["sensitivity"] = savedTouch.sensitivity;
        touchCfg["swap_xy"] = savedTouch.swap_xy;
        touchCfg["invert_x"] = savedTouch.invert_x;
        touchCfg["invert_y"] = savedTouch.invert_y;
        touchCfg["display_width"] = dc.width;
        touchCfg["display_height"] = dc.height;
        touchCfg["sda_pin"] = dc.i2c_sda;
        touchCfg["scl_pin"] = dc.i2c_scl;
        JsonObject calibration = touchCfg["calibration"].to<JsonObject>();
        calibration["x_min"] = savedTouch.calibration.x_min;
        calibration["x_max"] = savedTouch.calibration.x_max;
        calibration["y_min"] = savedTouch.calibration.y_min;
        calibration["y_max"] = savedTouch.calibration.y_max;
        calibration["swapped_xy"] = savedTouch.calibration.swapped_xy;
        const bool touchStarted = touchManager.begin(touchCfg);
        touchUiController.setBroadcastCallback([](const JsonObject& status) {
            wsServer.setUiSource("touch");
            wsServer.broadcastTelemetry(status);
        });
        touchUiController.setWifiHotspotCallback([]() {
            wifiMgr.startAP();
        });
        static UiApp ui_instance(touchUiController, touchManager, cycleHistory);
        uiApp = &ui_instance;
        uiApp->setWifiStatusCallback([]() {
            WifiSnapshot snapshot;
            snapshot.ssid = wifiMgr.getConfig().ssid;
            if (snapshot.ssid.isEmpty() && wifiMgr.isAPActive()) {
                snapshot.ssid = WifiManager::AP_SSID;
            }
            snapshot.rssi = wifiMgr.getRSSI();
            snapshot.connected = wifiMgr.isConnected();
            return snapshot;
        });
        const int8_t haptic_pin = configMgr.getActuatorConfig().custom_pin;
        if (haptic_pin >= 0) {
            pinMode(haptic_pin, OUTPUT);
            uiApp->setHapticCallback([]() {
                const int8_t pin = configMgr.getActuatorConfig().custom_pin;
                if (pin < 0) {
                    return;
                }
                digitalWrite(pin, HIGH);
                delayMicroseconds(1500);
                digitalWrite(pin, LOW);
            });
        }
        uint16_t w = dc.width > 0 ? dc.width : 320;
        uint16_t h = dc.height > 0 ? dc.height : 240;
        const bool uiStarted = uiApp->begin(w, h, &displayManager);
        g_touchUiOwnsDisplay =
            touchStarted && uiStarted && displayManager.getActiveDriver();
        xTaskCreatePinnedToCore(
            uiTask, "UiTask", 8192, nullptr, 3,
            &uiTaskHandle, 1);
    }
    
    logMgr.logSystem(LogLevel::INFO, LogModule::SYSTEM, 
                     "Filament Dryer ESP32 v%s started", FIRMWARE_VERSION);
    
}

void loop() {
    // Main loop is empty - all work done in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}