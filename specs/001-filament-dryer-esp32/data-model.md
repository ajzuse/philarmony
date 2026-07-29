# Data Model & State Machine Specification (Generic Architecture)

> **Tooling note (2026-07-28):** The root `Makefile` add-on introduces **no** runtime entities, NVS keys, or state-machine transitions. Developer-facing target contract lives in `contracts/makefile-targets.md`. Sections below remain the firmware data model of record.

## 1. System State Machine

```text
               +-----------------------+
               |         BOOT          |
               +-----------+-----------+
                           |
                           v
               +-----------------------+
               |     WIFI_CONNECT      |
               +-----+-----------+-----+
                     |           |
            (Success)|           |(Fail / No Credentials)
                     v           v
            +--------+--+     +--+--------+
            |  READY    |     | HOTSPOT   | ("philarmony" AP)
            +-----+-----+     +-----+-----+
                  |                 |
                  | (start)         | (config saved)
                  v                 v
            +-----+-----+     +-----+-----+
            |  DRYING   |<----+ REBOOT    |
            +--+-----+--+     +-----------+
               |     |
      (stop /  |     | (sensor error / over-temp /
     complete) |     |  thermal runaway)
               v     v
            +--+-----+--+
            |  STOPPED  | / FAULT_STOPPED
            +-----------+
```

### States
- `BOOT`: Initial hardware setup, mounting LittleFS, loading NVS config, driver registry init
- `WIFI_CONNECT`: Attempting connection to stored WiFi SSID
- `HOTSPOT`: Access Point active (SSID "philarmony", IP 192.168.4.1) serving HTTP config page
- `READY`: Connected to WiFi, WebSocket server active, hardware drivers initialized, system idle
- `DRYING`: Active drying session, PID/control loop running, telemetry streaming at 1Hz
- `STOPPED`: Session ended normally by user or target reached
- `FAULT_STOPPED`: Emergency stop triggered by safety supervisor. All actuators disabled (0% PWM), system/network/logs operational

---

## 2. Entities & Schemas

### 2.1 DryingSession
Represents an active or completed drying run.

| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| `session_id` | uint32 | Monotonic session ID | > 0 |
| `status` | string | "idle" \| "drying" \| "stopped" \| "fault_stopped" | enum |
| `profile_id` | string | ID of filament profile used (or "custom") | non-empty |
| `target_temp_c` | float | Target temperature in °C | 30.0 to 80.0 |
| `target_humidity_pct` | float | Target humidity % RH (optional, 0 = disabled) | 0.0 to 50.0 |
| `max_duration_min` | uint16 | Maximum drying duration in minutes | 1 to 1440 |
| `elapsed_sec` | uint32 | Elapsed duration in seconds | >= 0 |
| `remaining_sec` | uint32 | Remaining duration in seconds | >= 0 |
| `start_timestamp` | uint32 | Epoch or uptime start time | > 0 |
| `stop_reason` | string | "running" \| "completed" \| "user_stopped" \| "humidity_reached" \| "over_temp" \| "sensor_error" \| "thermal_runaway" | enum |

### 2.2 SensorConfig (Generic)
Configuration for temperature and humidity sensors.

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| `id` | string | Unique sensor identifier | "chamber_temp" |
| `type` | string | Sensor driver key | "sht3x", "dht22", "ds18b20", "ntc", "bme280", "custom" |
| `capabilities` | array[string] | Measured quantities | ["temperature", "humidity"] |
| `bus` | object | Bus configuration | see below |
| `driver_params` | object | Driver-specific parameters | {"resolution": 12} |
| `calibration` | object | Per-capability calibration | {"temperature_offset": 0.0} |

#### Bus Configuration Objects

**I2C:**
```json
{ "type": "i2c", "bus": 0, "address": 68, "sda_pin": 21, "scl_pin": 22 }
```

**SPI:**
```json
{ "type": "spi", "mosi_pin": 19, "sclk_pin": 18, "cs_pin": 5, "dc_pin": 16 }
```

**1-Wire:**
```json
{ "type": "onewire", "pin": 4 }
```

**UART:**
```json
{ "type": "uart", "rx_pin": 16, "tx_pin": 17, "baudrate": 9600 }
```

**ADC:**
```json
{ "type": "adc", "pin": 34, "attenuation": "11db" }
```

### 2.3 ActuatorConfig (Generic)
Configuration for heater MOSFET and exhaust fan.

| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| `id` | string | Unique actuator identifier | "heater", "exhaust_fan" |
| `type` | string | Driver type | "mosfet_pwm", "ssr", "fan_pwm", "fan_digital", "shared_mosfet", "custom" |
| `role` | string | Logical role | "heater", "fan", "custom" |
| `pins` | object | GPIO assignments | {"pwm": 25} |
| `control` | object | Control algorithm config | see below |
| `safety_limits` | object | Hardware protection limits | {"max_temp_c": 80} |

#### Control Object (Algorithm-Specific)

**PID:**
```json
{
  "algorithm": "pid",
  "pwm_freq_hz": 1000,
  "max_power_pct": 100,
  "parameters": { "kp": 12.5, "ki": 0.45, "kd": 32.1 }
}
```

**Bang-Bang (Hysteresis):**
```json
{
  "algorithm": "bang_bang",
  "parameters": { "hysteresis_c": 1.0 }
}
```

**PWM Feedforward:**
```json
{
  "algorithm": "pwm_feedforward",
  "pwm_freq_hz": 1000,
  "parameters": { "base_pwm_pct": 50, "temp_coefficient": 2.5 }
}
```

### 2.4 DisplayConfig (Generic)
Configuration for attached display hardware.

| Field | Type | Description | Values / Examples |
|-------|------|-------------|-------------------|
| `enabled` | bool | Enable display rendering | true / false |
| `driver` | string | Display driver key | "ssd1306", "sh1106", "st7789", "ili9341", "st7735", "gc9a01", "ili9488", "hd44780", "nextion", "auto" |
| `bus_type` | string | Interface bus | "i2c", "spi", "parallel_8bit", "uart" |
| `width` | uint16 | Horizontal pixel count | 128, 135, 240, 320, 480 |
| `height` | uint16 | Vertical pixel count | 32, 64, 128, 240, 320 |
| `rotation` | uint16 | Display orientation | 0, 90, 180, 270 |
| `spi_mosi` | int8 | SPI MOSI GPIO | 19, 13, 23 |
| `spi_sclk` | int8 | SPI SCK GPIO | 18, 14 |
| `spi_cs` | int8 | SPI Chip Select GPIO | 5, 15 |
| `dc_pin` | int8 | Data/Command GPIO | 16, 2 |
| `rst_pin` | int8 | Hardware Reset GPIO | 23, -1 |
| `backlight_pin` | int8 | PWM Backlight GPIO | 4, 21, -1 |
| `layout` | object | Field selection & scaling | see below |

#### Layout Object
```json
{
  "fields": ["chamber_temp_c", "target_temp_c", "humidity_pct", "heater_power_pct", "status"],
  "font_scaling": "auto",
  "compact_mode": false
}
```

### 2.5 ControlConfig (Generic)
Configuration for temperature control algorithm.

| Field | Type | Description | Default |
|-------|------|-------------|---------|
| `algorithm` | string | Control algorithm | "pid" |
| `parameters` | object | Algorithm-specific params | - |
| `auto_tune` | bool | Enable auto-tune | false |
| `safety_limits` | object | Integrated safety limits | - |

#### Algorithm Parameters

**PID:**
```json
{ "kp": 12.5, "ki": 0.45, "kd": 32.1 }
```

**Bang-Bang:**
```json
{ "hysteresis_c": 1.0 }
```

**PWM Feedforward:**
```json
{ "base_pwm_pct": 50, "temp_coefficient": 2.5 }
```

### 2.5 FilamentProfile
Filament preset template.

| Field | Type | Range / Example |
|-------|------|-----------------|
| `id` | string | "pla", "petg", "abs", "tpu", "nylon", "custom-1" |
| `name_pt` | string | "PLA Premium" |
| `name_en` | string | "PLA Premium" |
| `target_temp_c` | float | 50.0 |
| `default_duration_min` | uint16 | 240 |
| `target_humidity_pct` | float | 15.0 |
| `is_builtin` | bool | true |
| `created_at` | uint32 | 0 |
| `updated_at` | uint32 | 0 |

### 2.6 LogEntry
Formatted log entry line written to `system.log` or `drying.log`.

```text
[YYYY-MM-DD HH:MM:SS][LEVEL][MODULE] Message string
Exemplo:
[2026-07-23 14:30:15][INFO][DRYING] Session #1 started. Target: 50.0C, Duration: 120m
[2026-07-23 14:30:25][DATA][TELEMETRY] Temp: 28.5C, Target: 50.0C, Hum: 45.2%, HeaterPWM: 100%, FanPWM: 80%
[2026-07-23 15:10:02][ERROR][SAFETY] Thermal runaway detected on GPIO 25! Heater power cut off.
```