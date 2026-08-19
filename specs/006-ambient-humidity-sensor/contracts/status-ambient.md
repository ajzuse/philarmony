# Contract: Ambient Fields on Status / Control

**Feature**: `006-ambient-humidity-sensor`  
**Extends**: `specs/001-filament-dryer-esp32/contracts/websocket-api.md` (`status/update`)  
**Date**: 2026-08-19

## Additive `status/update` fields

| Field | Type | When present | Notes |
|-------|------|--------------|-------|
| `ambient_temp_c` | number \| null | Ambient configured | null if unavailable/error |
| `ambient_humidity_pct` | number \| null | Ambient configured | null if humidity invalid |
| `ambient_sensor_status` | string | Always recommended when feature built | `absent` \| `ok` \| `error` |
| `humidity_exhaust_blocked_by_ambient` | boolean | During drying when ambient gate evaluated | true if humidity-reduction suppressed by ambient |

Chamber fields unchanged:

- `chamber_temp_c`, `humidity_pct`, `target_humidity_pct` remain cycle/safety authority (FR-010).

## Example (ambient ok, ventilation useful)

```json
{
  "topic": "status/update",
  "payload": {
    "status": "drying",
    "chamber_temp_c": 49.2,
    "target_temp_c": 50.0,
    "humidity_pct": 28.0,
    "target_humidity_pct": 15.0,
    "ambient_temp_c": 24.1,
    "ambient_humidity_pct": 18.0,
    "ambient_sensor_status": "ok",
    "humidity_exhaust_blocked_by_ambient": false,
    "heater_on": true,
    "heater_power_pct": 40.0,
    "exhaust_fan_on": true,
    "exhaust_fan_power_pct": 80.0
  }
}
```

## Example (ambient blocks humidity exhaust)

Chamber RH within margin of ambient → humidity-reduction intent blocked. Thermal assist may still run the fan.

```json
{
  "topic": "status/update",
  "payload": {
    "status": "drying",
    "humidity_pct": 19.5,
    "target_humidity_pct": 15.0,
    "ambient_humidity_pct": 18.0,
    "ambient_sensor_status": "ok",
    "humidity_exhaust_blocked_by_ambient": true
  }
}
```

## Example (ambient absent)

```json
{
  "topic": "status/update",
  "payload": {
    "ambient_sensor_status": "absent",
    "ambient_temp_c": null,
    "ambient_humidity_pct": null,
    "humidity_exhaust_blocked_by_ambient": false
  }
}
```

## Client compatibility

- Control app (003) and other clients MUST ignore unknown keys (existing forward-compat rule).
- Display `layout.fields` MAY include `ambient_temp_c` / `ambient_humidity_pct` when ambient is configured.

## Control semantics (normative summary)

Documented in [data-model.md](../data-model.md) and [research.md](../research.md) R3:

- Ambient gate applies only to **humidity-reduction** exhaust intent.
- Mandatory cooldown exhaust is never cancelled by ambient.
- Ambient sensor fault → soft-fail to chamber-only humidity rules; no device-wide fault solely from ambient.
