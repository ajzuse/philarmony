# Data Model: Ambient Humidity Sensor Support

**Feature**: `006-ambient-humidity-sensor`  
**Date**: 2026-08-19  
**Extends**: `specs/001-filament-dryer-esp32/data-model.md`

## Entities

### AmbientSensorConfig (logical; mapped into SensorConfig)

Optional second temperature/humidity sensor for room/outside air.

| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| `placement` | string | Must be `ambient` on wire config | enum |
| `id` | string | Unique id (e.g. `ambient`) | unique among sensors |
| `type` | string | Driver key from shared catalog | humidity-capable catalog type |
| `capabilities` | array | MUST include `humidity` (typically also `temperature`) | min 1; humidity required for ambient role |
| `bus` | object | Same bus shapes as chamber | pin/address conflict checks |
| `calibration` | object | `temperature_offset/scale`, `humidity_offset/scale` | same as chamber |

**Relationships**: Independent of chamber sensor model (FR-001a). Absence = chamber-only mode.

**Flattened NVS fields** (additive on `SensorConfig`):

| Field | Type | Default |
|-------|------|---------|
| `ambient_enabled` | bool | false |
| `ambient_type` | string | empty |
| `ambient_i2c_bus` | uint8 | 0 |
| `ambient_i2c_address` | uint8 | 0 |
| `ambient_gpio_pin` | int8 | -1 |
| `ambient_sda_pin` / `ambient_scl_pin` | int8 | 21 / 22 |
| `ambient_temperature_offset/scale` | float | 0 / 1 |
| `ambient_humidity_offset/scale` | float | 0 / 1 |

### UselessVentilationMargin

| Field | Type | Description | Default |
|-------|------|-------------|---------|
| `useless_ventilation_margin_rh` | float | %RH gap below which exhaust is useless | 2.0 |
| `humidity_exhaust_hysteresis_rh` | float | Resume gap above margin | 1.0 (fixed v1) |

Stored under `control.parameters` when overridden; otherwise defaults apply.

### AmbientReading (runtime)

| Field | Type | Description |
|-------|------|-------------|
| `temp_c` | float / NaN | Calibrated ambient temperature |
| `humidity_pct` | float / NaN | Calibrated ambient humidity |
| `valid` | bool | Passed timeout/range/RoC |
| `status` | enum | `absent` \| `ok` \| `error` |

### HumidityExhaustDecision (runtime)

| Field | Type | Description |
|-------|------|-------------|
| `humidity_above_target` | bool | Chamber RH > cycle target |
| `ambient_gate_active` | bool | Ambient RH valid and gate enabled |
| `ventilation_useful` | bool | `chamber_rh > ambient_rh + margin` (+ hysteresis when resuming) |
| `humidity_exhaust_requested` | bool | Intent wants fan for humidity |
| `blocked_by_ambient` | bool | Requested but blocked by gate |

## State transitions

### Ambient sensor availability

```text
absent ──(config with placement=ambient)──► ok
ok ──(invalid beyond sensor_timeout_ms)──► error
error ──(valid reading restored)──► ok
* ──(ambient removed from config / disabled)──► absent
```

`error` and `absent` both disable ambient-aware stopping (chamber-only humidity-reduction rules). Neither alone enters `FAULT_STOPPED`.

### Humidity-reduction exhaust vs cooldown

```text
DRYING:
  humidity_exhaust_requested = (chamber_rh > target_humidity)
  if ambient status == ok:
    if currently exhausting for humidity:
      useful = chamber_rh > ambient_rh + margin
    else:
      useful = chamber_rh > ambient_rh + margin + hysteresis
    humidity_fan = humidity_exhaust_requested && useful
  else:
    humidity_fan = humidity_exhaust_requested   # chamber-only

  thermal_fan = existing thermal-assist rules
  fan_cmd = thermal_fan OR humidity_fan

COOLDOWN / safety cooldown:
  fan_cmd = mandatory cooldown duty  # ambient MUST NOT cancel
```

## Validation rules

1. Ambient type MUST be in supported humidity-capable catalog; unknown type → config error (same class as chamber).
2. Ambient without humidity capability → reject or treat humidity unavailable (plan: **reject at parse** with clear error).
3. Pin/I2C address conflicts with chamber/display/actuators → same conflict engine as 001.
4. Two ambient placements → config error (single ambient zone).
5. Zero ambient entries → valid; chamber-only.
6. Margin clamped to `[0, 10]` %RH if provided.

## Status payload fields (additive)

See [contracts/status-ambient.md](./contracts/status-ambient.md).

## Out of model scope (v1)

- Multi-room / multiple ambient zones
- Ambient-based cycle completion target
- Replacing chamber safety limits with ambient temperature
