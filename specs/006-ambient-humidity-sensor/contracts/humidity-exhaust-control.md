# Contract: Ambient-Aware Humidity Exhaust Control

**Feature**: `006-ambient-humidity-sensor`  
**Date**: 2026-08-19

## Purpose

Normative control rules for when humidity-reduction exhaust may run, given optional ambient humidity.

## Inputs

| Input | Source |
|-------|--------|
| `chamber_rh` | Chamber humidity sensor (calibrated) |
| `ambient_rh` | Ambient humidity when `ambient_sensor_status == ok` |
| `target_rh` | Active cycle `target_humidity_pct` |
| `margin` | `useless_ventilation_margin_rh` (default 2.0) |
| `hysteresis` | 1.0 %RH (v1 fixed) |
| `ambient_sensor_status` | `absent` \| `ok` \| `error` |

## Rules

1. **Cycle completion** still uses chamber target humidity / max time only (FR-010). Ambient never becomes the cycle success target.
2. **Humidity-reduction request** = drying active AND `chamber_rh` is valid AND `chamber_rh > target_rh`.
3. If `ambient_sensor_status != ok` OR ambient humidity invalid → humidity-reduction fan follows chamber-only request (no ambient block).
4. If ambient ok:
   - While humidity exhaust is **active**: allow while `chamber_rh > ambient_rh + margin`; else block.
   - While humidity exhaust is **inactive**: allow resume only when `chamber_rh > ambient_rh + margin + hysteresis`.
5. **Thermal assist** and **mandatory cooldown** fan intents are independent and are not blocked by rule 4.
6. Combined fan command = OR/max of thermal assist, allowed humidity-reduction, and cooldown intents (implementation may use max duty among active intents).
7. When humidity-reduction is blocked solely by ambient, set `humidity_exhaust_blocked_by_ambient = true` on status.
8. SC-001: once the blocked condition is stably met (after hysteresis/debounce consistent with actuator constraints), humidity-reduction duty from this intent reaches 0 within 5 seconds.

## Non-goals

- Using ambient temperature for heater PID or safety cutoffs
- Cancelling COOLDOWN because ambient is humid
- Requiring ambient hardware
