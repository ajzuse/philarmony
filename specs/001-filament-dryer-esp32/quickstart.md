# Quickstart & End-to-End Validation Guide (Generic Architecture)

This guide describes end-to-end scenarios to validate the Filament Dryer ESP32 firmware on actual target boards with **any compatible hardware** configured via generic JSON.

---

## 1. Prerequisites & Compilation

Preferred entrypoint is the root **Makefile** (see `contracts/makefile-targets.md`). PlatformIO remains underneath.

```bash
# Compile firmware (default ENV=esp32devkitc)
make build
make build ENV=lilygo_tdisplay_v1
make build ENV=esp32_2432s028

# Host Unity tests (must exit non-zero on failure)
make test

# Flash / install on connected ESP32
make flash                 # aliases: make upload, make install
make flash ENV=esp32devkitc PORT=/dev/ttyUSB0

# Optional: LittleFS + serial
make uploadfs
make monitor
make clean
make help
```

Equivalent raw PlatformIO (still valid):

```bash
# Build firmware with PlatformIO for target ESP32 board
pio run -e esp32devkitc          # ESP32 DevKitC V4
pio run -e lilygo_tdisplay_v1    # LilyGo T-Display V1.1
pio run -e esp32_2432s028        # ESP32-2432S028 CYD

# Upload firmware and SPIFFS/LittleFS filesystem
pio run -e esp32devkitc -t upload
pio run -e esp32devkitc -t uploadfs
```

### Supported Boards (Add more via `platformio.ini`)
| Environment | Board | Display | Notes |
|-------------|-------|---------|-------|
| `esp32devkitc` | ESP32-DevKitC V4 | External | Generic dev board |
| `lilygo_tdisplay_v1` | LilyGo T-Display V1.1 | ST7789 135x240 | Integrated display |
| `esp32_2432s028` | ESP32-2432S028 (CYD) | ILI9341 240x320 | Cheap Yellow Display |

---

## 2. Validation Scenario 1: Initial Hotspot Fallback & WiFi Setup

1. Erase WiFi credentials in NVS or boot with unconfigured ESP32.
2. Power on the ESP32.
3. Verify WiFi AP **"philarmony"** (password: **"philarmony"**) is broadcast within 5s.
4. Connect smartphone or PC to "philarmony" AP.
5. Open browser at `http://192.168.4.1`.
6. Fill local WiFi SSID and password, then click "Salvar e Conectar".
7. Verify ESP32 connects to target WiFi, disables hotspot "philarmony", and prints assigned local IP on serial/log.

---

## 3. Validation Scenario 2: Generic Hardware Configuration & Drying Cycle Test

### 3.1 Configure Hardware via WebSocket
Connect via WebSocket client (e.g. `wscat -c ws://<esp32_ip>/ws`):

```json
{
  "topic": "config/hardware",
  "payload": {
    "sensors": [
      {
        "id": "chamber_temp",
        "type": "sht3x",
        "capabilities": ["temperature", "humidity"],
        "bus": { "type": "i2c", "bus": 0, "address": 68, "sda_pin": 21, "scl_pin": 22 },
        "driver_params": { "repeatability": "high" },
        "calibration": { "temperature_offset": 0.0, "humidity_scale": 1.0 }
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
      "bus": { "type": "spi", "mosi": 19, "sclk": 18, "cs": 5, "dc": 16, "rst": 23, "bl": 4 },
      "geometry": { "width": 135, "height": 240, "rotation": 90 },
      "layout": { "fields": ["chamber_temp_c", "target_temp_c", "humidity_pct", "heater_power_pct", "status"], "font_scaling": "auto" }
    },
    "control": {
      "algorithm": "pid",
      "parameters": { "kp": 12.5, "ki": 0.45, "kd": 32.1 },
      "auto_tune": false,
      "safety_limits": { "hard_temp_limit_c": 80, "sensor_timeout_ms": 600 }
    }
  }
}
```

### 3.2 Subscribe to Status Stream
```json
{ "topic": "status/subscribe", "payload": {} }
```

### 3.3 Start Drying Cycle for PLA Profile (50°C, 120min)
```json
{ "topic": "control/start", "payload": { "profile_id": "pla" } }
```

### 3.4 Observe:
- WebSocket receives `status/update` every 1 second (1Hz ±20ms).
- MOSFET AOD4184 drives PWM to heater.
- SHT31 measures temperature rising toward 50°C.
- Display (if enabled) renders status fields at 1Hz without blocking control loop.

---

## 4. Validation Scenario 3: Safety Fault Injection & Log Retrieval Test

### 4.1 Inject Fault
1. Disconnect SHT31 sensor I2C wire during active drying cycle.

### 4.2 Expected Behavior
- Within 600ms (3 failed samples), the firmware detects sensor disconnect.
- Immediate safety shutdown: MOSFET AOD4184 PWM set to 0% (Heater OFF).
- System state transitions to `FAULT_STOPPED`.
- WebSocket receives `status/fault` event with reason `SENSOR_DISCONNECT`.
- **Crucial**: Platform, WiFi, HTTP server, and log endpoints **remain 100% online**.

### 4.3 Verify Log Endpoints
Open browser or `curl`:
```bash
curl http://<esp32_ip>/log/drying
curl http://<esp32_ip>/log/system
```

### 4.4 Verify Log Contents
- `drying.log` contains session start, telemetry, fault injection timestamp, and exact error message: `[ERROR][SAFETY] Sensor disconnect detected on I2C bus 0`.
- `drying.log` was **preserved** and **NOT deleted** during/after fault.

---

## 5. Validation Scenario 4: PID Auto-Tune Calibration

### 5.1 Start Auto-Tune
```json
{ "topic": "control/pid_calibrate", "payload": { "target_temp_c": 50.0, "cycles": 5 } }
```

### 5.2 Observe Progress
- WebSocket receives `status/pid_calibrate` updates:
```json
{ "topic": "status/pid_calibrate", "payload": { "status": "calibrating", "cycle": 3, "total_cycles": 5, "current_temp_c": 51.4, "kp": 14.2, "ki": 0.52, "kd": 36.8, "saved_to_nvs": false } }
```

### 5.3 Completion
- Final message:
```json
{ "topic": "status/pid_calibrate", "payload": { "status": "complete", "kp": 14.2, "ki": 0.52, "kd": 36.8, "saved_to_nvs": true } }
```
- Coefficients saved to NVS under `heater.pid` namespace.
- Subsequent drying cycles use auto-tuned PID.

---

## 6. Validation Scenario 5: Multi-Board Compatibility

| Board | Test | Expected Result |
|-------|------|-----------------|
| ESP32_DEVKITC_V4 | SHT31 (I2C) + AOD4184 (PWM) | WiFi → WebSocket → Config → Dry Cycle OK |
| LilyGo T-Display V1.1 | ST7789 Display + SHT31 | Display renders status fields at 1Hz |
| ESP32-2432S028 CYD | ILI9341 Display + SHT31 | Display renders, touch reserved for future |

---

## 7. Validation Scenario 6: Display Auto-Detection

1. Configure display as `"driver": "auto"` with bus pins.
2. Boot with ST7789 connected → System detects and initializes ST7789.
3. Boot with ILI9341 connected → System detects and initializes ILI9341.
4. Boot with SSD1306 connected → System detects and initializes SSD1306.

---

## 8. Validation Checklist

| Test | Pass Criteria |
|------|---------------|
| Hotspot AP starts | "philarmony" SSID visible within 5s |
| WiFi config saves | STA connects, AP disabled, WebSocket accessible |
| Generic hardware config | JSON accepted, drivers instantiated, pins validated |
| WebSocket 1Hz telemetry | Updates every 1000ms ±20ms |
| Display 1Hz render | Fields update, no control loop blocking |
| Drying cycle start/stop | Heater PWM responds, fan follows, state machine correct |
| Fault injection (sensor) | Heater OFF in <600ms, fault state, logs preserved |
| Log download HTTP | `/log/drying` and `/log/system` return correct files |
| PID auto-tune | Cycles complete, Kp/Ki/Kd calculated, saved to NVS |
| Profile management | CRUD via WebSocket, persistence across reboot |
| Multi-board build | `make build ENV=<env>` (or `pio run -e <env>`) succeeds for all environments |
| Makefile façade | `make help`, `make test`, and `make build` succeed; `make test` fails if any Unity case fails |
| Flash via Make | `make flash` / `upload` / `install` require an attached ESP32; without a device the command fails non-zero (expected) |