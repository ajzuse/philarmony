# DEC-014 — Optional Ambient Temperature/Humidity Sensor + Ambient-Aware Fan Stop

**Tags:** ambient, sensor, fan, humidity, ventilation, esp32  
**Feature:** `006-ambient-humidity-sensor`  
**Source:** `specs/006-ambient-humidity-sensor/research.md`  
**Updated:** 2026-09-24  
**Status:** Accepted

## Decision

1. Add an **optional ambient** temperature/humidity sensor using the existing `ISensorDriver` catalog and `DriverRegistry` factory — **no new driver code** (R1).
2. Extend the existing `SensorConfig` NVS struct with `ambient_*` fields (mirroring chamber fields) instead of a separate `AmbientConfig` struct/NVS key (R2).
3. Hardware config parser recognizes `"role": "ambient"` in a sensor object; missing/`"chamber"` role defaults to chamber (backward compatible). Ambient must declare `temperature`+`humidity` capabilities (R3).
4. Ambient-aware fan-stop check runs inside `controlLoopTask()`: when ambient is valid and `chamberHumidity <= ambientHumidity + margin`, override fan power to 0; otherwise leave chamber-only duty (R4).
5. Ambient timeout **30s** (`lastValidAmbientMs`); on expiry fall back to chamber-only fan logic with no device fault (R5).
6. Useless-ventilation margin is configurable in Settings, **0–10 %RH, default 2 %RH**, stored in `UISettings.ambient_ventilation_margin_pct` (R6).
7. Expose `ambient_temp_c`, `ambient_humidity_pct`, `ambient_sensor_type` on the WebSocket `status/update` payload; omitted when ambient is not configured, `null` on fallback (R7).
8. Touch UI: ambient readings on the monitoring screen (below chamber values) and ambient config + margin stepper on the settings/sensors screen (R8).
9. `DisplayManager::autoConfigureLayout()` adds `ambient_temp_c`/`ambient_humidity_pct` to `all_fields` for larger displays (R9).
10. Automated host/Unity flow tests (config parser, NVS round-trip, fan stop, timeout fallback, chamber-only) on this product spec; manual smoke → `005` (R10).

## Rationale

Chamber-only builds must remain fully valid; ambient sensing is an optional enhancement. The chamber sensor drives cycle control/safety unchanged (FR-002, FR-010, SC-002, SC-006). Reusing `ISensorDriver` avoids duplicative interfaces; a second instance is trivial (R1). Ambient-aware logic only affects humidity-reduction exhaust during DRYING — never the mandatory post-heater cooldown (spec Edge Cases).

## Alternatives rejected

| Option | Why rejected |
|--------|----------------|
| New `IAmbientSensorDriver` interface | Duplicative; the physical sensor is identical, only the role differs |
| Separate `AmbientConfig` struct / NVS key | New key + getter/setter + parser path; existing multi-role struct already suffices |
| New `"ambient"` capability instead of `"role"` | Capabilities describe what is measured, not placement |
| Nested `"ambient": {...}` WS object | Breaks flat payload convention; harder for clients |
| Configurable ambient timeout | 30s is a reasonable default; extra UI surface for marginal benefit |
| Event-driven fan stop | 50 Hz polling is sufficient for a 30s timeout |

## Consequences

- `SensorConfig` and `UISettings` gain ambient fields; NVS backward compatible (missing fields = unconfigured).
- New pure header `src/core/AmbientFanLogic.hpp` (`ambientIsAvailable`, `shouldStopFanForAmbient`, `clampAmbientMarginPct`) keeps the decision host-testable.
- `main.cpp` fan logic, telemetry (DRYING/READY + COOLDOWN), WebSocket payload, touch UI screens, and `DisplayManager` layout updated.
- Ambient calibration fields are range-validated/clamped (offset ±10, scale 0.5–2.0) per `data-model.md` §2.1.
- Pin/I2C-address conflicts with the chamber sensor are rejected at parse time.
