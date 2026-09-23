# Data Model: Ambient Humidity Sensor Support

**Feature**: `006-ambient-humidity-sensor` | **Date**: 2026-09-03
**Depends on**: `specs/001-filament-dryer-esp32/data-model.md` (SensorConfig, DryingSession)

## 1. Extended System State Machine

No changes to state transitions. Ambient sensor is read-only input to fan decision; it does not affect cycle state (READY/DRYING/PAUSED/STOPPED/COOLDOWN/FAULT).

```text
(unchanged from 001)
READY ──start──► DRYING ──stop──► COOLDOWN/STOPPED
                    │
               pause/resume
                    ▼
                  PAUSED
```

**Fan decision change** (within DRYING state only):

```text
Fan duty = resolveFanDutyPct(chamberTemp)
  └── if ambient_valid && chamberHumidity <= ambientHumidity + margin:
        fanPower = 0  (ventilation useless)
```

---

## 2. Entities

### 2.1 SensorConfig (NVS, extended)

Existing struct in `ConfigManager.hpp`. New fields added for ambient sensor:

| Field | Type | Default | Validation |
|-------|------|---------|------------|
| `ambient_type` | String | `""` (empty = not configured) | Must be in `DriverRegistry` catalog or empty |
| `ambient_i2c_address` | u8 | `0` | 0 = auto-probe |
| `ambient_gpio_pin` | i8 | `-1` | ≥ -1; -1 = not used |
| `ambient_sda_pin` | i8 | `21` | ≥ 0 |
| `ambient_scl_pin` | i8 | `22` | ≥ 0 |
| `ambient_temperature_offset` | float | `0.0f` | ±10.0 |
| `ambient_temperature_scale` | float | `1.0f` | 0.5–2.0 |
| `ambient_humidity_offset` | float | `0.0f` | ±10.0 |
| `ambient_humidity_scale` | float | `1.0f` | 0.5–2.0 |

**Persistence**: JSON serialization under NVS key `"sensor"` (existing). Backward compatible — missing ambient fields default to empty/unconfigured.

**Relationship**: `SensorConfig` ← 1:1 → ambient `ISensorDriver` instance (when `ambient_type` is non-empty).

### 2.2 UISettings (NVS, extended)

New field for configurable margin:

| Field | Type | Default | Validation |
|-------|------|---------|------------|
| `ambient_ventilation_margin_pct` | float | `2.0f` | 0.0–10.0 |

### 2.3 SensorReading (unchanged)

```cpp
struct SensorReading {
    bool valid = false;
    float temperature = NAN;
    float humidity = NAN;
    float pressure = NAN;
    uint32_t timestamp = 0;
    String error_message;
};
```

Ambient sensor uses same `SensorReading` struct. No extension needed.

### 2.4 Ambient Sensor State (runtime)

| Field | Type | Notes |
|-------|------|-------|
| `ambientSensor` | `ISensorDriver*` | nullptr when not configured |
| `ambientReading` | `SensorReading` | Latest calibrated reading |
| `lastValidAmbientMs` | `uint32_t` | Timestamp of last valid reading |
| `ambientAvailable` | `bool` | True if reading valid and <30s old |

---

## 3. Validation Rules (ambient-originated)

| Rule | Condition | Action |
|------|-----------|--------|
| Config parse | `"role": "ambient"` in sensor JSON | Accept; set `ambient_type` in SensorConfig |
| Config parse | `ambient_type` not in DriverRegistry | Reject with error message |
| Config parse | Pin conflict with chamber sensor | Reject with error message |
| Fan stop | `ambient_valid && chamberHumidity <= ambientHumidity + margin` | Fan power = 0 |
| Fallback | `ambientReading.valid == false` for >30s | Ignore ambient; use chamber-only rules |
| Margin range | `ambient_ventilation_margin_pct` outside 0–10 | Clamp to valid range |

---

## 4. Relationships

```text
SensorConfig (NVS)
    ├── chamber sensor → ISensorDriver (existing)
    └── ambient sensor → ISensorDriver (new, optional)

controlLoopTask (50Hz):
    ambientReading = ambientSensor->read()
    ambientAvailable = valid && (now - timestamp < 30s)
    fanDecision = ambientAvailable
        ? (chamberHumidity <= ambientHumidity + margin ? 0 : fanDuty)
        : fanDuty  // chamber-only fallback

status/update:
    ambient_temp_c = ambientReading.temperature
    ambient_humidity_pct = ambientReading.humidity
    ambient_sensor_type = sensorConfig.ambient_type
```
