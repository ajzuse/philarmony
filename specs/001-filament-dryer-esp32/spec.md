# Filament Dryer ESP32 - Base Structure Specification

## Feature Overview

**Feature Name**: Filament Dryer ESP32 Base Structure  
**Short Name**: filament-dryer-esp32  
**Version**: 0.1.0  
**Status**: Draft  
**Date**: 2026-07-23  

## Executive Summary

Implement the base firmware structure for an open-source DIY filament dryer running on ESP32. The device controls chamber temperature, humidity, drying time, heater, and exhaust ventilation. It provides WiFi connectivity with automatic fallback to a configuration hotspot, a WebSocket API for real-time control and monitoring, optional display support with configurable layouts, and per-second status updates via WebSocket and optional display.

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
3. User configures sensor types and GPIO pin mappings for heater/ventilation
4. User configures optional display (enabled/disabled, resolution, displayed fields)
5. User starts drying cycle with target temperature, max time, optional target humidity
6. System runs drying cycle, updating status every second via WebSocket
7. User can stop drying cycle at any time via interrupt command
8. Status updates include: current status, temperatures, heater status/power %, exhaust fan status, ESP32 CPU/memory usage

**Scenario 3: Display Operation**
1. Display is configured as enabled with specific resolution
2. User selects which status fields to display
3. During drying cycle, display updates every second with selected fields
4. Initial display shows same fields as WebSocket status stream

### Acceptance Criteria

| Scenario | Given | When | Then |
|----------|-------|------|------|
| Hotspot fallback | No WiFi configured or connection fails | ESP32 boots | Hotspot "philarmony"/"philarmony" active with HTTP config server on fixed IP |
| WiFi config | Connected to hotspot | User submits WiFi credentials via HTTP | ESP32 connects to WiFi, disables hotspot, starts WebSocket server |
| Sensor config | Connected via WebSocket | User sends sensor/pin config | Configuration persisted and applied to hardware |
| Display config | Connected via WebSocket | User sends display config | Display settings persisted and applied |
| Start drying | Configured sensors/pins | User sends start command with temp/time/humidity | Drying cycle starts, status streamed every second |
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
- Topics: `config/sensors`, `config/pins`, `config/display`, `control/start`, `control/stop`, `status/subscribe`, `status/unsubscribe`

### FR-003: Sensor and Pin Configuration
- Configure sensor types: temperature (DHT22, DS18B20, etc.), humidity (DHT22, etc.)
- Configure GPIO pins for: heater control (PWM), exhaust fan control (PWM/digital), sensor data pins
- Configuration persisted in non-volatile storage
- Validation: pin conflicts prevented, valid GPIO ranges enforced

### FR-004: Display Configuration
- Enable/disable display
- Configure display resolution (e.g., 128x64, 128x32, 240x240)
- Select which status fields to display (subset of WebSocket status fields)
- Configuration persisted and applied on boot

### FR-005: Drying Cycle Control
- Start command: target temperature (°C), maximum time (minutes), optional target humidity (%)
- Stop/interrupt command: immediately stops heater and ventilation
- Automatic stop when: max time reached, target humidity reached, safety temperature exceeded
- Safety: hard temperature limit (e.g., 80°C) cuts heater regardless of target

### FR-006: Real-time Status Streaming
- WebSocket topic `status/subscribe` enables per-second JSON updates
- Status payload includes:
  - `status`: "idle" | "drying" | "stopped" | "error"
  - `chamber_temp_c`: current chamber temperature
  - `target_temp_c`: target temperature
  - `humidity_pct`: current humidity
  - `target_humidity_pct`: target humidity (null if not set)
  - `heater_on`: boolean
  - `heater_power_pct`: 0-100 PWM duty cycle
  - `exhaust_fan_on`: boolean
  - `exhaust_fan_power_pct`: 0-100
  - `elapsed_time_sec`: seconds since cycle start
  - `remaining_time_sec`: seconds until max time (null if no limit)
  - `cpu_usage_pct`: ESP32 CPU utilization
  - `memory_free_bytes`: free heap memory
  - `uptime_sec`: device uptime

### FR-007: Display Updates
- If display enabled: update configured fields every second
- Initial display shows same fields as status stream
- Support for different resolutions and layouts
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
- Software temperature limit: heater off at 80°C chamber temp
- Fan runs for 30s after heater off to cool chamber

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

### SensorConfig
- `temperature_sensor`: { type: string, pin: number, ...params }
- `humidity_sensor`: { type: string, pin: number, ...params }

### PinConfig
- `heater_pwm_pin`: number
- `exhaust_fan_pwm_pin`: number
- `exhaust_fan_digital_pin`: number (optional)

### DisplayConfig
- `enabled`: boolean
- `width`: number
- `height`: number
- `driver`: "ssd1306" | "st7789" | "ili9341" | "auto"
- `fields`: array of field names from status payload

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
- All fields from FR-006

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

1. ESP32 variant: ESP32-WROOM-32 or ESP32-S3 (4MB flash minimum)
2. Sensors: DHT22 for temp/humidity, DS18B20 for temperature (user configurable)
3. Heater control: MOSFET + PWM on GPIO
4. Exhaust fan: PWM or digital control on GPIO
5. Display: I2C OLED (SSD1306) or SPI TFT (ST7789/ILI9341) - auto-detect or config
6. Power supply: 12V/24V for heater, 3.3V/5V for ESP32 and logic
7. No touch screen support in this phase
8. Single drying chamber (single temperature/humidity zone)
9. Configuration via WebSocket only (no local buttons/encoder in this phase)
10. Safety temperature limit: 80°C hardcoded (configurable in future)

## Dependencies & Constraints

- **Hardware**: ESP32 dev board, temperature/humidity sensors, MOSFET driver, exhaust fan, optional display
- **Software**: ESP-IDF or Arduino framework, AsyncWebServer/AsyncWebSocket libraries
- **Storage**: NVS for WiFi credentials and device config
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

### Configure Sensors
```json
{ "topic": "config/sensors", "payload": { "temperature": { "type": "dht22", "pin": 4 }, "humidity": { "type": "dht22", "pin": 4 } } }
```

### Configure Pins
```json
{ "topic": "config/pins", "payload": { "heater_pwm": 25, "exhaust_fan_pwm": 26, "exhaust_fan_digital": 27 } }
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

### Status Update (server → client, 1Hz)
```json
{ "topic": "status/update", "payload": { "status": "drying", "chamber_temp_c": 48.5, "target_temp_c": 50, "humidity_pct": 22.1, "target_humidity_pct": 20, "heater_on": true, "heater_power_pct": 65, "exhaust_fan_on": true, "exhaust_fan_power_pct": 80, "elapsed_time_sec": 3600, "remaining_time_sec": 3600, "cpu_usage_pct": 12.5, "memory_free_bytes": 245760, "uptime_sec": 7200 } }
```