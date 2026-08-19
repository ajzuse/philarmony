# WebSocket API Contract - Generic Hardware-Agnostic

## Protocol Overview
- **Protocol**: JSON over WebSocket
- **Port**: Default 80 (ws://<esp32_ip>/ws)
- **Framing**: `{"topic": "<topic_name>", "payload": { ... }}`

---

## 1. Commands (Client → ESP32)

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
- Either `profile_id` OR explicit parameters (`target_temp_c`, `max_duration_min`, `target_humidity_pct`) required
- If both provided, explicit parameters override profile

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
- Allowed from `drying`, `paused`, or `cooldown`

### 1.2b Pause / Resume (feature 004)
**Topic**: `control/pause`
```json
{ "topic": "control/pause", "payload": { "reason": "user_requested" } }
```
- Requires `status == drying`; heater off; elapsed frozen; auto-stop after 30 minutes (`pause_timeout`)

**Topic**: `control/resume`
```json
{ "topic": "control/resume", "payload": {} }
```
- Requires `status == paused`

See also: `specs/004-esp32-touchscreen-ui/contracts/pause-resume-api.md`

### 1.3 Subscribe / Unsubscribe Telemetry
**Topic**: `status/subscribe` / `status/unsubscribe`
```json
{
  "topic": "status/subscribe",
  "payload": {}
}
```

### 1.4 Hardware Configuration (Generic Klipper-Style)
**Topic**: `config/hardware`
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

### 1.6 Control Algorithm Configuration
**Topic**: `config/control`
```json
{
  "topic": "config/control",
  "payload": {
    "algorithm": "pid",
    "parameters": {
      "kp": 12.5,
      "ki": 0.45,
      "kd": 32.1
    },
    "auto_tune": false,
    "safety_limits": {
      "hard_temp_limit_c": 80.0,
      "max_heater_power_pct": 100,
      "sensor_timeout_ms": 600
    }
  }
}
```

### 1.7 PID Auto-Tune Calibration
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

### 1.8 Filament Profile Management
**Topic**: `config/profiles/list`
```json
{ "topic": "config/profiles/list", "payload": {} }
```

**Topic**: `config/profiles/get`
```json
{ "topic": "config/profiles/get", "payload": { "profile_id": "pla" } }
```

**Topic**: `config/profiles/create`
```json
{
  "topic": "config/profiles/create",
  "payload": {
    "name_pt": "Meu Filamento",
    "name_en": "My Filament",
    "target_temp_c": 55,
    "default_duration_min": 180,
    "target_humidity_pct": 12
  }
}
```

**Topic**: `config/profiles/update`
```json
{
  "topic": "config/profiles/update",
  "payload": {
    "profile_id": "custom-abc123",
    "target_temp_c": 60,
    "default_duration_min": 240
  }
}
```

**Topic**: `config/profiles/delete`
```json
{ "topic": "config/profiles/delete", "payload": { "profile_id": "custom-abc123" } }
```

**Topic**: `config/profiles/reset_defaults`
```json
{ "topic": "config/profiles/reset_defaults", "payload": {} }
```

---

## 2. Server Broadcasts (ESP32 → Client)

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

**Completion**:
```json
{
  "topic": "status/pid_calibrate",
  "payload": {
    "status": "complete",
    "kp": 14.2,
    "ki": 0.52,
    "kd": 36.8,
    "saved_to_nvs": true
  }
}
```

### 2.5 Profile List Response
**Topic**: `config/profiles/list/response`
```json
{
  "topic": "config/profiles/list/response",
  "payload": {
    "profiles": [
      { "id": "pla", "name_pt": "PLA", "name_en": "PLA", "target_temp_c": 50.0, "default_duration_min": 240, "target_humidity_pct": 15.0, "is_builtin": true },
      { "id": "petg", "name_pt": "PETG", "name_en": "PETG", "target_temp_c": 65.0, "default_duration_min": 240, "target_humidity_pct": 15.0, "is_builtin": true },
      { "id": "custom-abc123", "name_pt": "Meu Filamento", "name_en": "My Filament", "target_temp_c": 55.0, "default_duration_min": 180, "target_humidity_pct": 12.0, "is_builtin": false, "created_at": 1690123456, "updated_at": 1690123456 }
    ]
  }
}
```

### 2.6 Single Profile Response
**Topic**: `config/profiles/get/response`
```json
{
  "topic": "config/profiles/get/response",
  "payload": {
    "profile": {
      "id": "pla",
      "name_pt": "PLA",
      "name_en": "PLA",
      "target_temp_c": 50.0,
      "default_duration_min": 240,
      "target_humidity_pct": 15.0,
      "is_builtin": true
    }
  }
}
```

### 2.7 Profile Create/Update/Delete Response
```json
{
  "topic": "config/profiles/create/response",
  "payload": {
    "status": "created",
    "profile_id": "custom-xyz789"
  }
}
```

### 2.8 Hardware Configuration Response
```json
{
  "topic": "config/hardware/response",
  "payload": { "status": "saved" }
}
```

### 2.8 Error Response
```json
{
  "topic": "config/hardware/error",
  "payload": { "error": "Pin conflict: GPIO 25 already used by heater" }
}
```