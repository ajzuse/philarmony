# Contract: Ambient Sensor Hardware Config

**Feature**: `006-ambient-humidity-sensor`  
**Extends**: `specs/001-filament-dryer-esp32/contracts/config-schema.json` (`sensors[]`)  
**Date**: 2026-08-19

## Placement field

Each `sensors[]` item MAY include:

```json
"placement": "chamber" | "ambient"
```

| Value | Meaning |
|-------|---------|
| `chamber` (default if omitted) | Chamber control / safety / cycle humidity |
| `ambient` | Optional room/outside reference for humidity-reduction exhaust |

**Id aliases** (when `placement` omitted): `id` equal to `ambient` or matching `ambient_*` → treat as ambient.

## Ambient entry requirements

- `capabilities` MUST include `"humidity"`.
- `type` MUST be a humidity-capable driver from the shared catalog (same set as chamber): e.g. `sht3x` / `sht31`, `aht20`, `bme280`, `dht22`, …
- Chamber and ambient types MAY differ.
- At most one ambient-placed sensor.

## Example (dual sensor, different models)

```json
{
  "topic": "config/hardware",
  "payload": {
    "sensors": [
      {
        "id": "chamber",
        "placement": "chamber",
        "type": "dht22",
        "capabilities": ["temperature", "humidity"],
        "bus": { "type": "onewire", "pin": 4 },
        "calibration": { "humidity_offset": 0.0, "humidity_scale": 1.0 }
      },
      {
        "id": "ambient",
        "placement": "ambient",
        "type": "sht31",
        "capabilities": ["temperature", "humidity"],
        "bus": { "type": "i2c", "bus": 0, "address": 68, "sda_pin": 21, "scl_pin": 22 },
        "calibration": { "humidity_offset": 0.0, "humidity_scale": 1.0 }
      }
    ],
    "actuators": [],
    "control": {
      "algorithm": "pid",
      "parameters": {
        "kp": 12.5,
        "ki": 0.45,
        "kd": 32.1,
        "useless_ventilation_margin_rh": 2.0
      },
      "safety_limits": { "hard_temp_limit_c": 80, "sensor_timeout_ms": 600 }
    }
  }
}
```

(`actuators` omitted in snippet; real payloads must still satisfy heater requirements from 001.)

## Chamber-only (unchanged)

Omitting ambient (or no `placement: ambient`) MUST parse and behave as today.

## Errors

| Condition | Result |
|-----------|--------|
| Unknown ambient `type` | `config/hardware/error` (same class as chamber) |
| Ambient without humidity capability | Error |
| Second ambient placement | Error |
| Bus/pin/address conflict | Error (existing conflict engine) |

## Normative notes for 001 schema

Additive JSON Schema properties (do not remove existing ones):

- `sensors.items.properties.placement`: `{ "type": "string", "enum": ["chamber", "ambient"], "default": "chamber" }`
- `control.parameters.useless_ventilation_margin_rh`: number, default 2.0, minimum 0, maximum 10
