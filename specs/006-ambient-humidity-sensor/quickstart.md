# Quickstart: Ambient Humidity Sensor Support

**Feature**: `006-ambient-humidity-sensor`  
**Date**: 2026-08-19

## Prerequisites

- Firmware tree from repo root (PlatformIO / `make`)
- Familiarity with `specs/001-filament-dryer-esp32/quickstart.md`
- Contracts: [config-ambient.md](./contracts/config-ambient.md), [status-ambient.md](./contracts/status-ambient.md), [humidity-exhaust-control.md](./contracts/humidity-exhaust-control.md)

## Build & automated tests

```bash
make test
# or: pio test -e native
```

Expected: native suite passes, including new ambient flow tests (chamber-only unchanged, dual-sensor stop, ambient soft-fail).

```bash
make build
# or: pio run -e esp32devkitc
```

Expected: firmware builds with ambient parser/control paths linked.

## Validation scenarios

### VS-A1 — Chamber-only unchanged

1. Configure only chamber temp/humidity sensor (no ambient).
2. Start a drying cycle with target humidity.
3. **Expect**: Fan/cycle behavior matches pre-006 chamber-only rules; `ambient_sensor_status` is `absent`; no new mandatory config step.

### VS-A2 — Ambient blocks useless humidity exhaust

1. Configure chamber + ambient (any two humidity-capable catalog types; models may differ).
2. Simulate/inject chamber RH above target and within `margin` (default 2 %RH) of ambient RH (or ambient ≥ chamber).
3. Ensure thermal-assist intent is off (heater off / below thermal fan threshold) so humidity-reduction is the fan driver.
4. **Expect**: Within 5s, humidity-reduction exhaust stops; `humidity_exhaust_blocked_by_ambient` true; cycle still running until chamber target/time rules complete.

### VS-A3 — Ambient allows useful ventilation

1. Dual sensors; chamber RH above target and **above** ambient by more than margin.
2. **Expect**: Humidity-reduction exhaust may run; `humidity_exhaust_blocked_by_ambient` false.

### VS-A4 — Ambient fault soft-fail

1. Dual-sensor drying active with ambient ok.
2. Invalidate ambient (disconnect / timeout).
3. **Expect**: Within one `sensor_timeout_ms`, status shows `ambient_sensor_status=error` (or equivalent unavailable); ambient gate disabled; chamber-only humidity rules; no device-wide fault solely from ambient; chamber over-temp still cuts heater.

### VS-A5 — Cooldown not cancelled

1. Complete a cycle into mandatory exhaust cooldown with ambient humid (≥ chamber).
2. **Expect**: Cooldown fan still runs for configured cooldown duration; ambient gate does not cancel it.

### VS-A6 — Config reject unsupported ambient type

1. Send `config/hardware` with ambient `type` not in catalog.
2. **Expect**: `config/hardware/error` with clear message; prior valid config retained per 001 rules.

## Manual hardware smoke

Open manual dual-sensor smoke (when hardware available) MUST be recorded under `specs/005-manual-validation` (not as open manuals on this product spec). Suggested stage name: **VS-AMB-1 Dual-sensor ambient exhaust**.

## References

- Data model: [data-model.md](./data-model.md)
- Research decisions: [research.md](./research.md)
- Plan: [plan.md](./plan.md)
