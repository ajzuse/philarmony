# Contract: Ambient Sensor Configuration & Status API

**Feature**: `006-ambient-humidity-sensor`
**Normative peer**: `specs/001-filament-dryer-esp32/contracts/websocket-api.md`

## 1. Hardware Config JSON — Ambient Sensor

A sensor object with `"role": "ambient"` in the hardware config JSON array:

```json
{
  "type": "sht31",
  "role": "ambient",
  "capabilities": ["temperature", "humidity"],
  "i2c_address": "0x44",
  "sda_pin": 16,
  "scl_pin": 17,
  "temperature_offset": 0.0,
  "humidity_offset": 0.0
}
```

**Rules**:
- At most one ambient sensor per device
- `type` MUST be in the supported sensor catalog (same as chamber)
- `capabilities` MUST include `"humidity"` (ambient without humidity is ignored for fan decisions)
- Pin conflicts with chamber sensor MUST be rejected at parse time
- Missing `"role"` or `"role": "chamber"` → treated as chamber sensor (backward compatible)

## 2. NVS Persistence — SensorConfig

```json
{
  "type": "sht31",
  "is_integrated": true,
  "i2c_bus": 0,
  "i2c_address": 68,
  "sda_pin": 21,
  "scl_pin": 22,
  "ambient_type": "sht31",
  "ambient_i2c_address": 69,
  "ambient_sda_pin": 16,
  "ambient_scl_pin": 17,
  "ambient_temperature_offset": 0.0,
  "ambient_humidity_offset": 0.0
}
```

**Backward compatibility**: Missing `ambient_*` fields → ambient not configured. Existing single-sensor configs are unaffected.

## 3. NVS Persistence — UISettings

```json
{
  "brightness_pct": 80,
  "timeout_sec": 120,
  "ambient_ventilation_margin_pct": 2.0
}
```

## 4. WebSocket status/update — Ambient Fields

Added to existing `status/update` payload when ambient sensor is configured:

| Field | Type | Values |
|-------|------|--------|
| `ambient_temp_c` | float | Latest calibrated ambient temperature (°C) |
| `ambient_humidity_pct` | float | Latest calibrated ambient humidity (%RH) |
| `ambient_sensor_type` | string | Sensor type string (e.g. `"sht31"`) |

**Omission**: Fields omitted entirely when ambient sensor not configured.

**Example** (ambient configured):
```json
{
  "status": "drying",
  "chamber_temp_c": 52.3,
  "humidity_pct": 18.5,
  "ambient_temp_c": 24.1,
  "ambient_humidity_pct": 55.2,
  "ambient_sensor_type": "sht31"
}
```

**Example** (ambient not configured):
```json
{
  "status": "drying",
  "chamber_temp_c": 52.3,
  "humidity_pct": 18.5
}
```

## 5. Fan Stop Decision — Ambient-Aware Logic

**Input**: `chamberHumidity`, `ambientHumidity`, `margin` (0–10%RH, default 2%RH)

| Condition | Fan Action |
|-----------|------------|
| `ambient_valid && chamberHumidity <= ambientHumidity + margin` | Fan power = 0 (ventilation useless) |
| `ambient_valid && chamberHumidity > ambientHumidity + margin` | Fan power = normal duty (ventilation useful) |
| `!ambient_valid` (not configured, invalid, or >30s timeout) | Fan power = normal duty (chamber-only fallback) |

**Timing**: Evaluated every control loop tick (50Hz). Fan stops within 5s of condition being stably met (SC-001).

## 6. Touch UI — Ambient Display

**Monitoring screen**: Ambient temperature and humidity labels displayed below chamber values (when configured). Format: `"Amb: 24.1°C / 55.2%"`.

**Settings > Sensors screen**: Shows ambient sensor type, I2C address, SDA/SCL pins (read-only).

**Omission**: Ambient labels hidden when sensor not configured.

## 7. Fallback Protocol

When ambient sensor fails or times out (>30s without valid reading):

1. Set `ambientAvailable = false`
2. Fan decision reverts to chamber-only (existing behavior)
3. Touch UI shows ambient as `"--"` or `"N/A"`
4. WebSocket sends `ambient_temp_c: null, ambient_humidity_pct: null`
5. No device fault raised (ambient is optional)
6. If ambient recovers (valid reading received), `ambientAvailable = true` resumes ambient-aware logic
