# Quickstart & End-to-End Validation Guide

This guide describes end-to-end scenarios to validate the Filament Dryer ESP32 firmware on actual target boards (`ESP32_DEVKITC_V4`, `LilyGo T-Display V1.1`, `ESP32-2432S028 CYD`) with `SHT31` sensor and `MOSFET AOD4184`.

---

## 1. Prerequisites & Compilation

```bash
# Build firmware with PlatformIO for target ESP32 board
pio run -e esp32devkitc

# Upload firmware and SPIFFS/LittleFS filesystem
pio run -e esp32devkitc -t upload
pio run -e esp32devkitc -t uploadfs
```

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

## 3. Validation Scenario 2: SHT31 & MOSFET AOD4184 Drying Cycle Test

1. Connect via WebSocket client (e.g. `wscat -c ws://<esp32_ip>/ws`).
2. Send hardware configuration for SHT31 (I2C SDA=21, SCL=22) and MOSFET AOD4184 (GPIO 25):
   ```json
   {
     "topic": "config/hardware",
     "payload": {
       "sensor": { "type": "sht31", "sda_pin": 21, "scl_pin": 22, "i2c_address": 68 },
       "actuators": { "heater_pin": 25, "heater_pwm_freq": 1000, "fan_mode": "independent_pwm", "fan_pin": 26 }
     }
   }
   ```
3. Subscribe to status stream:
   ```json
   { "topic": "status/subscribe", "payload": {} }
   ```
4. Start drying cycle for PLA profile (50°C, 120min):
   ```json
   { "topic": "control/start", "payload": { "profile_id": "pla" } }
   ```
5. Observe:
   - WebSocket receives `status/update` every 1 second (1Hz ±20ms).
   - MOSFET AOD4184 drives PWM to heater.
   - SHT31 measures temperature rising toward 50°C.

---

## 4. Validation Scenario 3: Safety Fault Injection & Log Retrieval Test

1. **Inject Fault**: Disconnect SHT31 sensor I2C wire during active drying cycle.
2. **Expected Behavior**:
   - Within 600ms (3 failed samples), the firmware detects sensor disconnect.
   - Immediate safety shutdown: MOSFET AOD4184 PWM set to 0% (Heater OFF).
   - System state transitions to `FAULT_STOPPED`.
   - WebSocket receives `status/fault` event with reason `SENSOR_DISCONNECT`.
   - **Crucial**: Platform, WiFi, HTTP server, and log endpoints **remain 100% online**.
3. **Verify Log Endpoints**:
   - Open browser or `curl`:
     ```bash
     curl http://<esp32_ip>/log/drying
     curl http://<esp32_ip>/log/system
     ```
   - Verify `drying.log` contains session start, telemetry, fault injection timestamp, and exact error message: `[ERROR][SAFETY] Sensor disconnect detected on I2C bus 0`.
   - Verify `drying.log` was preserved and NOT deleted during/after fault.
