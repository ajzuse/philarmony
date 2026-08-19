# Data Model: Filament Dryer Control App

**Date**: 2026-08-10  
**Storage**: Drift (SQLite) on-device + in-memory telemetry buffers  
**Amended**: 2026-08-10 — Clarifications (`KnownDevice` locked; Stop-only; background session policy)  
**Related**: `packages/philarmony_core` models; firmware WS payloads in `001` contracts

## Naming note

| Name | Owner | Meaning |
|------|-------|---------|
| `DeviceProfile` (existing) | `philarmony_core` / installer | Flash/wizard hardware+WiFi profile |
| `KnownDevice` | Control app | Saved connection target (nickname, host, port) |
| `FilamentProfile` | Shared core + device sync | Material drying preset |
| `HardwareConfig` | WS / core DTO | Live `config/hardware` payload |

---

## Entities

### KnownDevice (local registry)

| Field | Type | Rules |
|-------|------|-------|
| id | UUID | PK |
| nickname | string | 1–64 chars |
| host | string | IP or hostname |
| port | int | default **80** (001); user override allowed |
| path | string | default `/ws` |
| deviceModel | string? | from device when known |
| firmwareVersion | string? | semver string from device |
| lastSeen | DateTime? | |
| lastConnected | DateTime? | |
| autoConnect | bool | default false (one preferred true) |
| notificationSettings | JSON | per-event toggles |

**Relationships**: 1 → N `DryingCycle`; 0..1 active `DeviceSession` (runtime only).

### DeviceSession (runtime, not persisted)

| Field | Type | Notes |
|-------|------|-------|
| knownDeviceId | UUID | |
| connectionState | enum | connecting / connected / disconnected / error |
| lastError | string? | |
| reconnectAttempt | int | exponential backoff, max interval 60s |
| status | StatusSnapshot? | last `status/update` |
| fault | FaultEvent? | last `status/fault` |
| hardwareCache | HardwareConfig? | last successful get/ack |
| profilesCache | FilamentProfile[] | from `config/profiles/list` |

**State transitions (connection)**:
```
idle → connecting → connected ⇄ reconnecting → disconnected
                 ↘ error
```

### StatusSnapshot (from `status/update`)

Mirrors firmware payload: `status`, `chamber_temp_c`, `target_temp_c`, `humidity_pct`, `target_humidity_pct`, `heater_on`, `heater_power_pct`, `exhaust_fan_on`, `exhaust_fan_power_pct`, `elapsed_time_sec`, `remaining_time_sec`, `cpu_usage_pct`, `memory_free_bytes`, `uptime_sec`.

**Validation**: tolerate missing optional fields; mark sensor error badge when temp/humidity NaN/null while status drying.

### DryingCycle (persisted)

| Field | Type | Rules |
|-------|------|-------|
| id | UUID | PK |
| knownDeviceId | UUID | FK |
| profileId | string? | filament profile id |
| materialName | string | user tag / profile name |
| targetTempC | double | 30–80 |
| maxDurationSec | int | from start command |
| targetHumidityPct | double? | 5–50 |
| startTime | DateTime | |
| endTime | DateTime? | null while active |
| avgTempC / maxTempC | double? | computed |
| avgHumidityPct | double? | |
| stopReason | enum | completed, stopped, target_humidity, max_time, safety_cutoff, error, power_loss |
| notes | string | |
| hasSamples | bool | detail charts available |

### CycleSample (persisted, optional dense)

| Field | Type | Rules |
|-------|------|-------|
| cycleId | UUID | FK |
| tSec | int | elapsed |
| tempC | double | |
| humidityPct | double? | |
| heaterPct | double | |
| fanPct | double | |

**Policy**: buffer live at 1Hz in memory; on cycle end write downsampled series (e.g. ≤1 sample / 5s or max 2000 points).

### FilamentProfile (shared + device)

Reuse `philarmony_core.FilamentProfile`: `id`, `namePt`, `nameEn`, `targetTempC`, `defaultDurationMin`, `targetHumidityPct`, `isBuiltin`, timestamps for customs.

**Validation** (existing validator): temp 30–80°C, duration 1–1440 min, humidity 5–50%.

**Sync**: device is source of truth when online; offline edits → `PendingCommand`.

### HardwareConfig / DisplayConfig / Pin mapping

Align with 001 `config/hardware` (+ display/control subsections). Client-side validation via pin conflict checks before send; surface `config/hardware/error` to UI.

### PendingCommand (offline queue)

| Field | Type | Rules |
|-------|------|-------|
| id | UUID | |
| knownDeviceId | UUID | |
| topic | string | e.g. `control/start`, `config/hardware` |
| payloadJson | string | |
| createdAt | DateTime | |
| status | enum | queued / sending / acked / failed |
| lastError | string? | |

**Conflict policy**: device wins for config; history merges by timestamp (FR-010).

### AppPreferences (local)

locale (`pt_BR`/`en_US`), temp unit (°C/°F), time format, optional PIN hash, maxConcurrentSessions (default 3), chart window minutes (default 60).

---

## Control commands (validated before send)

### StartCycleRequest

- Either `profileId` **or** explicit `targetTempC` + `maxDurationMin` (+ optional `targetHumidityPct`)
- Explicit fields override profile when both present (001)
- Bounds enforced client-side; device may still reject

### StopCycleRequest

- `reason`: `user_requested` (default)
- UI confirmation required (FR-003)
- **No Pause/Resume** commands in MVP (out of scope until firmware documents them)

---

## Background session (runtime policy)

When `status` indicates an active drying cycle (or user enabled “keep monitoring”), mobile `BackgroundSession` MUST keep the WS alive per FR-007a. Record `backgroundAllowed: bool` on session for UX when OS denies.

---

## Entity relationship (logical)

```text
KnownDevice 1──* DryingCycle 1──* CycleSample
KnownDevice 1──* PendingCommand
DeviceSession (runtime) ──1 KnownDevice
FilamentProfile ← synced via WS; cached on session + optional local draft
```
