# Data Model & State Machine Specification

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
- `BOOT`: Initial hardware setup, mounting LittleFS, loading NVS config.
- `WIFI_CONNECT`: Attempting connection to stored WiFi SSID.
- `HOTSPOT`: Access Point active (SSID "philarmony", IP 192.168.4.1) serving HTTP config page.
- `READY`: Connected to WiFi, WebSocket server active, system idle.
- `DRYING`: Active drying session, PID heater control running, telemetry streaming at 1Hz.
- `STOPPED`: Session ended normally by user or target reached.
- `FAULT_STOPPED`: Emergency stop triggered by safety supervisor. Heater disabled (0% PWM), system/network/logs operational.

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

### 2.2 SensorConfig
Configuration for temperature and humidity sensors.

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| `type` | string | Sensor driver key ("sht31", "dht22", "ds18b20", "ntc_thermistor", "bme280") | "sht31" |
| `is_integrated` | bool | True if temp & humidity are in same physical IC | true |
| `i2c_bus` | uint8 | I2C bus number (0 or 1) | 0 |
| `i2c_address` | uint8 | I2C address in hex | 0x44 |
| `gpio_pin` | int8 | GPIO pin for 1-Wire, DHT, or ADC sensor (-1 if I2C) | -1 |
| `sda_pin` | int8 | I2C SDA GPIO | 21 |
| `scl_pin` | int8 | I2C SCL GPIO | 22 |

### 2.3 ActuatorConfig
Configuration for heater MOSFET and exhaust fan.

| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| `heater_pin` | int8 | GPIO pin for Heater MOSFET PWM | Valid ESP32 output GPIO |
| `heater_pwm_freq` | uint32 | PWM frequency in Hz | 100 to 5000 |
| `heater_max_power_pct` | uint8 | Soft safety power limit % | 10 to 100 |
| `fan_mode` | string | "shared_mosfet" \| "independent_pwm" \| "independent_digital" | enum |
| `fan_pin` | int8 | GPIO pin for Fan MOSFET/PWM | Valid ESP32 output GPIO |
| `fan_pwm_freq` | uint32 | Fan PWM frequency in Hz | 100 to 25000 |
| `cooldown_duration_sec`| uint16| Post-heating fan run time in seconds | 0 to 300 |

### 2.4 DisplayConfig
Configuration for attached display hardware.

| Field | Type | Description | Values / Examples |
|-------|------|-------------|-------------------|
| `enabled` | bool | Enable display rendering loop | true / false |
| `driver` | string | Display controller driver key | "ssd1306", "sh1106", "st7789", "ili9341", "st7735", "gc9a01", "ili9488", "hd44780", "nextion", "none" |
| `bus_type` | string | Interface bus | "i2c", "spi", "parallel_8bit", "uart" |
| `width` | uint16 | Horizontal pixel count | 128, 135, 240, 320, 480 |
| `height` | uint16 | Vertical pixel count | 32, 64, 128, 240, 320 |
| `rotation` | uint16 | Display orientation in degrees | 0, 90, 180, 270 |
| `spi_mosi` | int8 | SPI MOSI GPIO (-1 if I2C) | 19, 13, 23 |
| `spi_sclk` | int8 | SPI SCK GPIO (-1 if I2C) | 18, 14 |
| `spi_cs` | int8 | SPI Chip Select GPIO | 5, 15 |
| `dc_pin` | int8 | Data/Command GPIO | 16, 2 |
| `rst_pin` | int8 | Hardware Reset GPIO | 23, -1 |
| `backlight_pin` | int8 | PWM Backlight GPIO | 4, 21, -1 |

### 2.5 FilamentProfile
Filament preset template.

| Field | Type | Range / Example |
|-------|------|-----------------|
| `id` | string | "pla", "petg", "abs", "tpu", "nylon", "custom-1" |
| `name_pt` | string | "PLA Premium" |
| `name_en` | string | "PLA Premium" |
| `target_temp_c` | float | 50.0 |
| `default_duration_min`| uint16 | 240 |
| `target_humidity_pct` | float | 15.0 |
| `is_builtin` | bool | true |

### 2.5 LogEntry
Formatted log entry line written to `system.log` or `drying.log`.

```text
[YYYY-MM-DD HH:MM:SS][LEVEL][MODULE] Message string
Exemplo:
[2026-07-23 14:30:15][INFO][DRYING] Session #1 started. Target: 50.0C, Duration: 120m
[2026-07-23 14:30:25][DATA][TELEMETRY] Temp: 28.5C, Target: 50.0C, Hum: 45.2%, HeaterPWM: 100%, FanPWM: 80%
[2026-07-23 15:10:02][ERROR][SAFETY] Thermal runaway detected on GPIO 25! Heater power cut off.
```
