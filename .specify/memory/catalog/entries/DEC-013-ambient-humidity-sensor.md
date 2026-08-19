# DEC-013: Ambient Humidity Sensor & Exhaust Gate

**Tags:** sensors, humidity, fan-control, firmware  
**Source:** `specs/006-ambient-humidity-sensor/research.md`  
**Updated:** 2026-08-19

## Decision

Optional second temp/humidity sensor with `placement: ambient` (shared catalog; model need not match chamber). Ambient humidity gates **humidity-reduction** exhaust only (margin default 2 %RH, hysteresis 1 %RH). Soft-fail to chamber-only on ambient loss. Cooldown/thermal intents unchanged. Status adds `ambient_*` fields.

## Rejected

- Ambient as cycle completion target
- Gating cooldown/thermal fan by ambient
- Ambient-only exotic drivers / requiring matching chamber model
- Blocking 006 on control-app/installer UI polish
