# WebSocket API Contract

## Protocol Overview
- **Protocol**: JSON over WebSocket
- **Port**: Default 80 (ws://<esp32_ip>/ws)
- **Framing**: `{"topic": "<topic_name>", "payload": { ... }}`

---

## 1. Commands (Client -> ESP32)

### 1.1 Start Drying Session
**Topic**: `control/start`
```json
{
  "topic": "control/start",
  "payload": {
    "profile_id": "pla",
    "target_temp_c": 50.0,
    "max_duration_min": 240,
    "target_humidity_pct": 15.0
  }
}
```

### 1.2 Stop Drying Session
**Topic**: `control/stop`
```json
{
  "topic": "control/stop",
  "payload": {
    "reason": "user_requested"
  }
}
```

### 1.3 Subscribe / Unsubscribe Telemetry
**Topic**: `status/subscribe` / `status/unsubscribe`
```json
{
  "topic": "status/subscribe",
  "payload": {}
}
```

### 1.4 Hardware & Sensor Configuration (Klipper Style)
**Topic**: `config/hardware`
```json
{
  "topic": "config/hardware",
  "payload": {
    "sensor": {
      "type": "sht31",
      "i2c_bus": 0,
      "sda_pin": 21,
      "scl_pin": 22,
      "i2c_address": 68
    },
    "actuators": {
      "heater_pin": 25,
      "heater_pwm_freq": 1000,
      "fan_mode": "independent_pwm",
      "fan_pin": 26,
      "fan_pwm_freq": 5000
    }
  }
}
```

### 1.5 Display Configuration
**Topic**: `config/display`
```json
{
  "topic": "config/display",
  "payload": {
    "enabled": true,
    "driver": "st7789",
    "bus_type": "spi",
    "width": 135,
    "height": 240,
    "rotation": 90,
    "spi_mosi": 19,
    "spi_sclk": 18,
    "spi_cs": 5,
    "dc_pin": 16,
    "rst_pin": 23,
    "backlight_pin": 4,
    "fields": ["chamber_temp_c", "target_temp_c", "humidity_pct", "heater_power_pct", "status"]
  }
}
```

### 1.6 Start PID Calibration
**Topic**: `control/pid_calibrate`
```json
{
  "topic": "control/pid_calibrate",
  "payload": {
    "target_temp_c": 50.0,
    "cycles": 5
  }
}
```

---

## 2. Server Broadcasts (ESP32 -> Client)

### 2.1 1Hz Real-Time Telemetry Broadcast
**Topic**: `status/update`
```json
{
  "topic": "status/update",
  "payload": {
    "status": "drying",
    "chamber_temp_c": 49.2,
    "target_temp_c": 50.0,
    "humidity_pct": 18.5,
    "target_humidity_pct": 15.0,
    "heater_on": true,
    "heater_power_pct": 42.5,
    "exhaust_fan_on": true,
    "exhaust_fan_power_pct": 80.0,
    "elapsed_time_sec": 1240,
    "remaining_time_sec": 13160,
    "cpu_usage_pct": 14.2,
    "memory_free_bytes": 224500,
    "uptime_sec": 3800
  }
}
```

### 2.2 System Fault Notification
**Topic**: `status/fault`
```json
{
  "topic": "status/fault",
  "payload": {
    "fault_code": "THERMAL_RUNAWAY",
    "message": "Heater power >80% for 45s with no temperature increase.",
    "action_taken": "Heater MOSFET PWM cut off to 0%. Platform online.",
    "timestamp_sec": 3812
  }
}
```

### 2.3 Real-Time Log Stream
**Topic**: `logs/stream`
```json
{
  "topic": "logs/stream",
  "payload": {
    "target_log": "drying",
    "line": "[2026-07-23 14:35:00][INFO][DRYING] Temperature reached target 50.0C. Regulating PWM."
  }
}
```

### 2.4 PID Calibration Progress & Result
**Topic**: `status/pid_calibrate`
```json
{
  "topic": "status/pid_calibrate",
  "payload": {
    "status": "calibrating",
    "cycle": 3,
    "total_cycles": 5,
    "current_temp_c": 51.4,
    "kp": 14.2,
    "ki": 0.52,
    "kd": 36.8,
    "saved_to_nvs": true
  }
}
```
