# Filament Dryer ESP32 - Base Structure Specification

## Feature Overview

**Feature Name**: Filament Dryer ESP32 Base Structure  
**Short Name**: filament-dryer-esp32  
**Version**: 0.1.0  
**Status**: Draft  
**Date**: 2026-07-23  

## Clarifications

### Session 2026-07-23
- Q: What is the behavior when target humidity is reached before max time? → A: Option A (Immediately complete cycle, turn off heater, and run fan for 30s cooldown)
- Q: Should a PID auto-tuning routine (PID_CALIBRATE) be supported? → A: Option A (Implement automated PID Auto-Tune via WebSocket control/pid_calibrate and store calculated Kp, Ki, Kd in NVS)

## Executive Summary

Implement the base firmware structure for an open-source DIY filament dryer running on ESP32. The device controls chamber temperature, humidity, drying time, heater, and exhaust ventilation. It provides WiFi connectivity with automatic fallback to a configuration hotspot, a WebSocket API for real-time control and monitoring, optional display support with configurable layouts, and per-second status updates via WebSocket and optional display.

**Key Design Principle**: **100% Configurable & Generic Architecture** - No hardcoded hardware references. All sensors, actuators, displays, and control algorithms are configured via JSON/YAML. The firmware loads driver implementations dynamically based on configuration. Supports any sensor, actuator, display, or control algorithm through a plugin/driver architecture.

## User Scenarios & Testing

### Primary User Scenarios

**Scenario 1: Initial Setup via Hotspot**
1. User powers on ESP32 for the first time
2. ESP32 fails to connect to configured WiFi
3. ESP32 creates hotspot "philarmony" (password: "philarmony") with fixed IP
4. User connects to hotspot and accesses HTTP server
5. User configures target WiFi credentials
6. ESP32 connects to configured WiFi and disables hotspot

**Scenario 2: Normal Operation via WebSocket**
1. ESP32 connects to configured WiFi
2. User connects via WebSocket to ESP32 IP
3. User configures sensor types, GPIO pin mappings, and control algorithms via generic configuration API
4. User configures optional display (enabled/disabled, resolution, displayed fields)
5. User starts drying cycle with target temperature, max time, optional target humidity
6. System runs drying cycle, updating status every second via WebSocket
7. User can stop drying cycle at any time via interrupt command
8. Status updates include: current status, temperatures, heater status/power %, exhaust fan status, ESP32 CPU/memory usage

**Scenario 3: Display Operation**
1. Display is configured as enabled with specific resolution and driver
2. User selects which status fields to display
3. During drying cycle, display updates every second with selected fields
4. Initial display shows same fields as WebSocket status stream

**Scenario 4: Hardware Configuration (Generic)**
1. User defines sensor configuration via JSON (type, communication bus, pins, parameters)
2. User defines actuator configuration via JSON (type, control method, pins, parameters)
3. User defines display configuration via JSON (driver type, bus, pins, layout)
4. User defines control algorithm configuration (PID parameters, safety limits, calibration)
5. System loads appropriate drivers and validates configuration at boot

### Acceptance Criteria

| Scenario | Given | When | Then |
|----------|-------|------|------|
| Hotspot fallback | No WiFi configured or connection fails | ESP32 boots | Hotspot "philarmony"/"philarmony" active with HTTP config server on fixed IP |
| WiFi config | Connected to hotspot | User submits WiFi credentials via HTTP | ESP32 connects to WiFi, disables hotspot, starts WebSocket server |
| Hardware config | Connected via WebSocket | User sends generic hardware config | Configuration persisted and applied to hardware drivers |
| Display config | Connected via WebSocket | User sends display config | Display settings persisted and applied |
| Start drying | Configured sensors/pins/algorithms | User sends start command with temp/time/humidity | Drying cycle starts, status streamed every second |
| Stop drying | Drying cycle running | User sends stop command | Heater/ventilation stopped, status updated |
| Status stream | WebSocket connected | Every second during operation | JSON status with all required fields sent |
| Display update | Display configured | Every second during operation | Display shows configured fields with current values |

### Edge Cases

- WiFi credentials invalid: Hotspot reactivates after timeout
- Sensor read failure: Status reports error, drying continues with safety limits
- Display not connected but configured: System continues without display errors
- WebSocket disconnect during drying: Drying continues, status resumes on reconnect
- Target humidity reached before max time: Drying stops automatically
- Over-temperature safety: Heater cuts off regardless of target
- Actuator failure: Safety engine triggers emergency stop
- Calibration routine: Auto-tunes PID for any heater configuration

## Functional Requirements

### FR-001: WiFi Connection with Hotspot Fallback
- ESP32 attempts to connect to stored WiFi credentials on boot
- If connection fails or no credentials stored: activate hotspot SSID "philarmony" password "philarmony"
- Hotspot runs HTTP server on fixed IP (192.168.4.1) serving WiFi configuration page
- Configuration page accepts SSID/password and stores them persistently
- On successful WiFi connection: disable hotspot, start WebSocket server

### FR-002: WebSocket API Server
- WebSocket server starts when connected to WiFi
- Supports multiple concurrent clients
- Message format: JSON with `topic` and `payload` fields
- Core topics: `config/hardware`, `config/control`, `config/display`, `control/start`, `control/stop`, `status/subscribe`, `status/unsubscribe`, `control/pid_calibrate`

### FR-003: Generic Hardware Configuration (Sensors, Actuators, Displays)
- **Sensors**: Configure any sensor type via generic schema:
  - Type identifier (driver name): `sht3x`, `dht22`, `ds18b20`, `ntc`, `bme280`, `aht20`, `custom`
  - Communication bus: I2C (bus, address, pins), SPI (pins), 1-Wire (pin), UART (pins), ADC (pin)
  - Measurement capabilities: temperature, humidity, pressure, custom
  - Driver-specific parameters: resolution, sampling rate, calibration coefficients
  - Supports single integrated sensor (temp+humidity) OR separate sensors
- **Actuators**: Configure any actuator via generic schema:
  - Heater: MOSFET/SSR/PWM, pin, frequency, max power limit, control algorithm (PID, bang-bang, hystereis)
  - Fan: PWM/digital/shared MOSFET, pin, frequency, cooldown duration, speed curve
  - Custom actuators: generic GPIO/PWM/servo/stepper with custom parameters
- **Displays**: Configure any display via generic schema:
  - Driver: `ssd1306`, `sh1106`, `st7789`, `ili9341`, `st7735`, `gc9a01`, `ili9488`, `hd44780`, `nextion`, `auto`
  - Bus: I2C, SPI, parallel 8-bit, UART
  - Resolution, rotation, pin mapping
  - Layout engine: auto-adapts fields to resolution, configurable font scaling
- **Persistence**: All hardware configuration in NVS (survives power loss)
- **Validation**: Pin conflicts prevented, valid GPIO ranges enforced, bus conflicts detected
- **Hot-reload**: Configuration changes applied without reboot where possible

### FR-004: Display Configuration & Layout Engine
- Enable/disable display
- Configure display resolution (auto-detected or manual: 128x64, 128x32, 135x240, 240x240, 240x320, 320x480, 16x2, 20x4)
- Select which status fields to display (subset of WebSocket status fields)
- Auto-layout engine: adjusts font size, field positioning, density based on resolution
- Configurable refresh rate (1Hz - 5Hz, default 1Hz)
- Configuration persisted and applied on boot
- No touch screen differentiation in this phase

### FR-005: Drying Cycle Control
- Start command: target temperature (°C), maximum time (minutes), optional target humidity (%)
- Stop/interrupt command: immediately stops heater and ventilation
- Automatic stop when: max time reached, target humidity reached, safety temperature exceeded
- On completion (target humidity or max time reached): immediately turn off heater PWM and run exhaust fan for 30 seconds cooldown before marking status as "stopped"
- Safety: hard temperature limit (configurable, default 80°C) cuts heater regardless of target
- Configurable safety limits per heater type

### FR-006: Real-time Status Streaming
- WebSocket topic `status/subscribe` enables per-second JSON updates
- Status payload includes all configured sensor readings, actuator states, and system metrics
- Payload structure is generic and reflects configured sensors/actuators

### FR-007: Display Updates
- If display enabled: update configured fields every second
- Initial display shows same fields as status stream
- Support for different resolutions and layouts via layout engine
- No touch screen differentiation in this phase

### FR-008: Filament Drying Profiles (Material Templates)
- Built-in default profiles for common filaments: PLA (50°C, 4h, 15% RH), PETG (65°C, 4h, 15% RH), ABS (80°C, 2h, 10% RH), TPU (45°C, 4h, 20% RH), Nylon (70°C, 6h, 10% RH)
- Each profile defines: target temperature (°C), default duration (minutes), target humidity (% RH), display name (PT-BR/EN-US)
- User can create, edit, delete custom profiles via WebSocket
- Profiles stored in NVS (persists across reboots), max 20 custom profiles
- Start drying command accepts either explicit parameters OR profile ID
- WebSocket topics: `config/profiles/list`, `config/profiles/get`, `config/profiles/create`, `config/profiles/update`, `config/profiles/delete`, `config/profiles/reset_defaults`
- Profile validation: temp 30-80°C, duration 1-1440 min, humidity 5-50%
- Built-in profiles are read-only (cannot be deleted, but can be overridden by custom profile with same ID)

### FR-009: Automated PID Calibration Routine (PID_CALIBRATE)
- WebSocket command `control/pid_calibrate` initiates Ziegler-Nichols thermal auto-tuning cycle for a given target temperature (e.g. 50°C)
- System cycles heater PWM output to measure thermal inertia, heating rate, and cooling delay
- Calculates optimal PID coefficients: `Kp` (proportional), `Ki` (integral), `Kd` (derivative)
- Saves calculated `Kp`, `Ki`, `Kd` values persistently to NVS upon calibration completion
- Emits real-time calibration progress and final PID parameters via WebSocket topic `status/pid_calibrate`
- Works with any heater configuration (MOSFET, SSR, different thermal masses)

### FR-010: Generic Control Algorithm Framework
- Supports multiple control algorithms: PID, Bang-Bang (hysteresis), PWM with feedforward, custom algorithms
- Algorithm selected and parameterized via configuration
- Auto-tune routine works with any algorithm that exposes PID-like parameters
- Safety engine integrates with all algorithms (hard limits, sensor validation, watchdog)

### FR-011: Safety & Fault Tolerance (Generic)
- Hardware watchdog timer enabled
- Configurable safety limits per actuator type
- Sensor validation: timeout, range, rate-of-change checks
- Thermal runaway detection (configurable thresholds)
- I2C/SPI bus lockup detection and recovery
- Emergency stop: immediate actuator cutoff, system stays online for diagnostics
- Fault codes: standardized, extensible

## Non-Functional Requirements

### Performance
- WebSocket status updates: exactly 1Hz ±100ms
- WebSocket command response: <100ms
- Hotspot HTTP server response: <500ms
- Sensor read cycle: <50ms per sensor
- Maximum RAM usage: <4MB (ESP32 limit)

### Safety
- Hardware watchdog timer enabled
- Heater over-temperature hardware cutoff (if supported by hardware)
- Software temperature limit: heater off at configurable limit (default 80°C)
- Fan runs for configurable duration after heater off to cool chamber

### Reliability
- Configuration persisted in NVS (survives power loss)
- Automatic WiFi reconnection with exponential backoff
- WebSocket auto-reconnect for clients
- Display driver resilient to I2C/SPI errors

### Usability
- Hotspot config page in Portuguese and English
- WebSocket API documented in Portuguese and English
- Display text in Portuguese (configurable)

## Key Entities

### DryingCycle
- `id`: unique cycle identifier
- `target_temp_c`: number (required)
- `max_duration_sec`: number (required)
- `target_humidity_pct`: number (optional)
- `start_time`: timestamp
- `end_time`: timestamp (null if running)
- `stop_reason`: "completed" | "stopped" | "target_humidity" | "max_time" | "safety_cutoff" | "error"

### SensorConfig (Generic)
- `id`: unique identifier
- `type`: driver type string (e.g., "sht3x", "dht22", "ds18b20", "ntc", "custom")
- `capabilities`: array of measured quantities ["temperature", "humidity", "pressure"]
- `bus`: object with bus-specific config (I2C/SPI/1Wire/UART/ADC)
- `driver_params`: object with driver-specific parameters
- `calibration`: offset/scale factors per capability

### ActuatorConfig (Generic)
- `id`: unique identifier
- `type`: driver type string (e.g., "mosfet_pwm", "ssr", "fan_pwm", "fan_digital", "stepper", "servo", "custom")
- `role`: "heater" | "fan" | "custom"
- `pins`: object with pin assignments
- `control`: object with algorithm-specific parameters
- `safety_limits`: object with min/max/rate limits

### DisplayConfig (Generic)
- `enabled`: boolean
- `driver`: driver type string
- `bus`: bus configuration (I2C/SPI/parallel/UART)
- `geometry`: width, height, rotation
- `pin_mapping`: bus-specific pin assignments
- `layout`: field selection, font scaling, layout mode (auto/manual)

### ControlConfig (Generic)
- `algorithm`: "pid" | "bang_bang" | "pwm_feedforward" | "custom"
- `parameters`: algorithm-specific parameters
- `auto_tune`: enable/disable auto-tune
- `safety_limits`: integrated with SafetyEngine

### FilamentProfile
- `id`: string (builtin: pla, petg, abs, tpu, nylon; custom: user-defined UUID)
- `name_pt`: string (Portuguese display name)
- `name_en`: string (English display name)
- `target_temp_c`: number (30-80)
- `default_duration_min`: number (1-1440)
- `target_humidity_pct`: number (5-50)
- `is_builtin`: boolean (read-only, true for default profiles)
- `created_at`: timestamp (for custom profiles)
- `updated_at`: timestamp (for custom profiles)

### StatusPayload
- Generic structure reflecting all configured sensors and actuators
- System metrics: cpu_usage_pct, memory_free_bytes, uptime_sec
- Drying session state if active

## Success Criteria

| Metric | Target | Measurement |
|--------|--------|-------------|
| Hotspot activation time | <5s after boot failure | Boot to HTTP server ready |
| WiFi connection time | <15s with valid credentials | Credentials stored to IP assigned |
| WebSocket latency | <100ms command response | Ping-pong test |
| Status update frequency | 1Hz ±100ms | Timestamp delta between messages |
| Memory usage | <3.5MB heap | ESP.getFreeHeap() during operation |
| CPU usage | <50% average | ESP.getCpuFreqMHz() monitoring |
| Display update sync | Within 200ms of status | Visual verification |
| Safety cutoff response | <100ms | Temp sensor to heater off |

## Assumptions

1. ESP32 variant: ESP32-WROOM-32, ESP32-S3, ESP32-C3 (4MB flash minimum)
2. Any sensor/actuator/display supported via driver registry
3. Configuration via WebSocket/HTTP only (no local buttons/encoder in this phase)
4. Single drying chamber (single temperature/humidity zone)
5. Safety temperature limit: configurable (default 80°C)
6. Power supply: 12V/24V for heater, 3.3V/5V for ESP32 and logic
7. No touch screen support in this phase
8. Open-source GPLv3 licensing

## Dependencies & Constraints

- **Hardware**: ESP32 dev board, any compatible sensors, MOSFET driver, exhaust fan, optional display
- **Software**: ESP-IDF or Arduino framework, AsyncWebServer/AsyncWebSocket, ArduinoJson, LittleFS
- **Storage**: NVS for WiFi credentials and device config, LittleFS for logs
- **Network**: 2.4GHz WiFi only (ESP32 limitation)
- **Constitutional**: Must comply with safety standards for DIY equipment, GPLv3 licensing, PT-BR/EN-US documentation

## Out of Scope (This Phase)

- Touch screen interaction
- Multi-zone drying chambers
- Mobile app / web dashboard (WebSocket API only)
- OTA firmware updates
- Multiple user accounts / authentication
- Historical data logging / export
- Scheduler / delayed start
- Cloud connectivity / MQTT

## Appendix: WebSocket Message Examples

### Generic Hardware Configuration
```json
{
  "topic": "config/hardware",
  "payload": {
    "sensors": [
      {
        "id": "chamber_temp",
        "type": "sht3x",
        "capabilities": ["temperature", "humidity"],
        "bus": { "type": "i2c", "bus": 0, "address": 0x44, "sda_pin": 21, "scl_pin": 22 },
        "driver_params": { "repeatability": "high" },
        "calibration": { "temperature_offset": 0.0, "humidity_scale": 1.0 }
      },
      {
        "id": "external_temp",
        "type": "ds18b20",
        "capabilities": ["temperature"],
        "bus": { "type": "onewire", "pin": 4 },
        "driver_params": { "resolution": 12 }
      }
    ],
    "actuators": [
      {
        "id": "heater",
        "type": "mosfet_pwm",
        "role": "heater",
        "pins": { "pwm": 25 },
        "control": { "algorithm": "pid", "pwm_freq_hz": 1000, "max_power_pct": 100 },
        "safety_limits": { "max_temp_c": 80, "max_power_pct": 100 }
      },
      {
        "id": "exhaust_fan",
        "type": "fan_pwm",
        "role": "fan",
        "pins": { "pwm": 26 },
        "control": { "pwm_freq_hz": 5000, "cooldown_sec": 30 },
        "safety_limits": { "max_power_pct": 100 }
      }
    ],
    "display": {
      "enabled": true,
      "driver": "st7789",
      "bus": { "type": "spi", "mosi": 19, "sclk": 18, "cs": 5, "dc": 16, "rst": 23 },
      "geometry": { "width": 135, "height": 240, "rotation": 1 },
      "layout": { "fields": ["chamber_temp_c", "target_temp_c", "humidity_pct", "heater_power_pct", "status"], "font_scaling": "auto" }
    },
    "control": {
      "algorithm": "pid",
      "parameters": { "kp": 12.5, "ki": 0.45, "kd": 32.1 },
      "auto_tune": false,
      "safety_limits": { "hard_temp_limit_c": 80 }
    }
  }
}
```

### Configure Display
```json
{ "topic": "config/display", "payload": { "enabled": true, "width": 128, "height": 64, "driver": "ssd1306", "fields": ["chamber_temp_c", "humidity_pct", "heater_power_pct", "status"] } }
```

### List Filament Profiles
```json
{ "topic": "config/profiles/list", "payload": {} }
```

### Get Filament Profile
```json
{ "topic": "config/profiles/get", "payload": { "profile_id": "pla" } }
```

### Create Custom Filament Profile
```json
{ "topic": "config/profiles/create", "payload": { "name_pt": "Meu Filamento", "name_en": "My Filament", "target_temp_c": 55, "default_duration_min": 180, "target_humidity_pct": 12 } }
```

### Update Filament Profile
```json
{ "topic": "config/profiles/update", "payload": { "profile_id": "custom-abc123", "target_temp_c": 60, "default_duration_min": 240 } }
```

### Delete Filament Profile
```json
{ "topic": "config/profiles/delete", "payload": { "profile_id": "custom-abc123" } }
```

### Reset to Built-in Defaults
```json
{ "topic": "config/profiles/reset_defaults", "payload": {} }
```

### Start Drying (with profile)
```json
{ "topic": "control/start", "payload": { "profile_id": "pla" } }
```

### Start Drying (custom parameters)
```json
{ "topic": "control/start", "payload": { "target_temp_c": 50, "max_time_min": 120, "target_humidity_pct": 20 } }
```

### Stop Drying
```json
{ "topic": "control/stop", "payload": {} }
```

### Subscribe Status
```json
{ "topic": "status/subscribe", "payload": {} }
```

### PID Auto-Tune Calibration
```json
{ "topic": "control/pid_calibrate", "payload": { "target_temp_c": 50.0, "cycles": 5 } }
```

### Status Update (server → client, 1Hz)
```json
{ "topic": "status/update", "payload": { "status": "drying", "chamber_temp_c": 49.2, "target_temp_c": 50.0, "humidity_pct": 18.5, "target_humidity_pct": 15.0, "heater_on": true, "heater_power_pct": 42.5, "exhaust_fan_on": true, "exhaust_fan_power_pct": 80.0, "elapsed_time_sec": 1240, "remaining_time_sec": 13160, "cpu_usage_pct": 14.2, "memory_free_bytes": 224500, "uptime_sec": 3800 } }
```

### PID Calibration Progress/Result
```json
{ "topic": "status/pid_calibrate", "payload": { "status": "calibrating", "cycle": 3, "total_cycles": 5, "current_temp_c": 51.4, "kp": 14.2, "ki": 0.52, "kd": 36.8, "saved_to_nvs": true } }
```

### Fault Notification
```json
{ "topic": "status/fault", "payload": { "fault_code": "THERMAL_RUNAWAY", "message": "Heater power >80% for 45s with no temperature increase.", "action_taken": "Heater MOSFET PWM cut off to 0%. Platform online.", "timestamp_sec": 3812 } }
```

### Real-Time Log Stream
```json
{ "topic": "logs/stream", "payload": { "target_log": "drying", "line": "[2026-07-23 14:35:00][INFO][DRYING] Temperature reached target 50.0C. Regulating PWM." } }
```