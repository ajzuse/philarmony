# Research: Ambient Humidity Sensor Support

**Feature**: `006-ambient-humidity-sensor`  
**Date**: 2026-08-19  
**Spec**: [spec.md](./spec.md)

## R1 — How to identify ambient vs chamber sensors in config

**Decision**: Add optional `placement` on each `sensors[]` entry: `"chamber"` | `"ambient"`. Default when omitted: `"chamber"` (backward compatible). Parser also accepts id aliases `ambient`, `ambient_*` as ambient when `placement` is absent.

**Rationale**: Explicit placement is unambiguous for DIY configs and installer UIs; id aliases avoid breaking hand-written configs that already use role-like ids. Matching chamber/ambient models is never required (FR-001a).

**Alternatives considered**:
- Id-only convention (`ambient_*`) — fragile, easy to misconfigure.
- Separate top-level `ambient_sensor` object — duplicates catalog schema and diverges from Klipper-style `sensors[]`.
- Require identical hardware models — rejected by clarify session 2026-08-19.

## R2 — Persistence / runtime representation

**Decision**: Extend flattened `SensorConfig` with ambient fields (`ambient_type`, bus pins/address, calibration offsets/scales, `ambient_enabled`) while continuing to accept full `sensors[]` on the wire. `HardwareConfigParser` maps the ambient-placed entry into those fields. Runtime keeps a second optional `ISensorDriver*` for ambient (same DriverRegistry factories as chamber).

**Rationale**: Matches existing 001 NVS flatten path (primary + optional humidity split + `extra_temp_*`) with minimal churn. Full multi-sensor runtime array is a larger 001 refactor and out of scope for v1 of this feature.

**Alternatives considered**:
- First-class `std::vector<SensorInstance>` only — cleaner long-term, invasive for ConfigManager/NVS/main loop.
- Reuse `extra_temp_*` for ambient — insufficient (needs humidity + explicit role; would overload meaning).

## R3 — Fan intents and ambient gate

**Decision**: Treat exhaust as three intents; ambient gates **only** humidity-reduction:

| Intent | When | Ambient gate? |
|--------|------|----------------|
| Thermal assist | Heater active or chamber temp > existing threshold (~40°C) / speed curve | No |
| Humidity-reduction | Chamber RH > cycle target humidity | Yes — stop when `chamber_rh <= ambient_rh + margin` |
| Mandatory cooldown | Post-completion / safety cooldown | No (never cancelled by ambient) |

Final fan power = max/OR of active intents. Resume humidity-reduction when `chamber_rh > ambient_rh + margin + hysteresis` (hysteresis default **1.0 %RH**) to prevent chatter (SC-001 stable condition).

**Rationale**: Spec separates humidity-reduction exhaust from safety/cooldown. Current firmware ties fan mainly to thermal assist + cooldown; v1 **adds** humidity-reduction intent when chamber humidity is above target, then applies the ambient usefulness check so the fan does not run uselessly against wet ambient air.

**Alternatives considered**:
- Gate all fan activity by ambient — would break thermal circulation and cooldown (violates FR cooldown assumption).
- Only document ambient without new humidity-reduction intent — would not satisfy User Story 1 when fan is otherwise off.
- Use ambient as cycle completion target — forbidden by FR-010.

## R4 — Useless-ventilation margin configuration

**Decision**: Default margin **2.0 %RH** (FR-004). Persist under `control.parameters.useless_ventilation_margin_rh` (float, clamp 0–10). v1 may omit advanced UI and ship the default; parser accepts override when present. Fixed hysteresis **1.0 %RH** (not user-facing in v1).

**Rationale**: Spec default is explicit; small config hook enables DIY tuning without new surfaces. Hysteresis addresses edge-case chatter without another user-facing knob.

**Alternatives considered**:
- Hard-code only — harder to tune on real hardware.
- Expose full UI in control app/installer in same milestone — deferred; firmware is source of truth (spec Assumptions).

## R5 — Status / API surface

**Decision**: Extend `status/update` (and display field allow-list) with:

- `ambient_temp_c` (number|null)
- `ambient_humidity_pct` (number|null)
- `ambient_sensor_status`: `"absent"` | `"ok"` | `"error"`
- `humidity_exhaust_blocked_by_ambient` (bool) — true when humidity-reduction intent is suppressed by ambient gate

Chamber fields (`chamber_temp_c`, `humidity_pct`) remain authoritative for cycle/safety.

**Rationale**: FR-006 requires distinguishable ambient readings; soft-fail visibility without inventing a device-wide fault (FR-007).

**Alternatives considered**:
- Nested `ambient: { temp, humidity, status }` — clearer but breaks flat status convention used by 003 codecs; flat fields are forward-compatible with “ignore unknown” clients.
- Reuse `extra_temp_*` status keys — ambiguous.

## R6 — Fault / soft-fail behavior

**Decision**: Ambient uses the same validation class as other sensors (timeout, range, RoC) but ambient failure **never** alone triggers `FAULT_STOPPED` / heater cut. After `sensor_timeout_ms` without valid ambient humidity → `ambient_sensor_status=error`, clear live ambient RH for control, disable ambient gate (chamber-only humidity-reduction rules). Chamber sensor faults retain existing SafetyEngine behavior.

**Rationale**: FR-007 soft-fail; SC-004 / SC-006.

**Alternatives considered**:
- Treat ambient like chamber for safety — violates optional-hardware rule.
- Ignore ambient faults silently with no status — operators could not diagnose (fails User Story 4).

## R7 — Catalog reuse

**Decision**: Ambient accepts the same humidity-capable types from DEC-003 / DriverRegistry as chamber (sht3x/sht30/sht31, aht20, bme280, dht22, and any future catalog humidity types). Types without humidity capability cannot satisfy ambient humidity role → treat as unavailable humidity (chamber-only fan logic) and reject at config if capabilities omit humidity when placement is ambient.

**Rationale**: FR-009 / FR-001a; no ambient-only exotic drivers.

## R8 — Downstream apps (002 / 003) scope for this plan

**Decision**: v1 implementation scope is **firmware (001 tree) + contracts under 006** (and additive contract notes). Control app / installer MUST remain forward-compatible (ignore unknown status keys; accept optional ambient sensor in `sensors[]` when sending hardware config). Dedicated UI polish for ambient configuration/display may be follow-up tasks tracked when those surfaces are updated; not blocking firmware Done-When.

**Rationale**: Spec Assumptions; keeps plan focused; avoids blocking 006 on Flutter/installer UI work.

## R9 — Testing placement

**Decision**: Automated native/flow tests live on this product spec (`test/` under firmware). Any open **manual** hardware smoke for dual-sensor builds is catalogued under `specs/005-manual-validation` (constitution v1.0.0).

**Rationale**: Catálogo Único de Validações Manuais.

## Memory catalog

Recorded as `DEC-013-ambient-humidity-sensor` in `.specify/memory/catalog/entries/`.
