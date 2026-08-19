# Data Model: ESP32 Touchscreen UI

**Feature**: `004-esp32-touchscreen-ui` | **Date**: 2026-08-19  
**Depends on**: `specs/001-filament-dryer-esp32/data-model.md` (`DryingSession`, `FilamentProfile`)

## 1. Extended System State Machine

```text
READY ──start──► DRYING ◄──resume── PAUSED
                     │                  ▲
                     │ pause            │
                     └──────────────────┘
                     │
              stop / complete / fault
                     ▼
              COOLDOWN / STOPPED / FAULT_STOPPED
```

| State | Heater | Fan (MVP) | Elapsed timer | UI |
|-------|--------|-----------|---------------|-----|
| READY | off | off | n/a | Home |
| DRYING | control loop | as control | running | Monitoring |
| **PAUSED** | **off** | **off** | **frozen** | Monitoring (paused) |
| COOLDOWN | off | cooldown policy | session ended | Monitoring/Cooldown |
| STOPPED / FAULT_STOPPED | off | off | stopped | Home / fault |

**New transitions** (must update `StateMachine` validity matrix):
- DRYING → PAUSED (user pause)
- PAUSED → DRYING (user resume)
- PAUSED → STOPPED / COOLDOWN (user stop — same stop path as from DRYING)
- PAUSED → FAULT_STOPPED (safety)

`DryingStopReason`: unchanged for stop; pause is **not** a stop reason (session still active).

---

## 2. Entities

### 2.1 TouchConfig (NVS)

| Field | Type | Validation |
|-------|------|------------|
| `controller_type` | enum | `auto`, `xpt2046`, `stmpe610`, `ft6236`, `gt911`, `cst816s`, `none` |
| `i2c_address` | u8 | optional; 0 = probe |
| `spi_cs` | i8 | ≥ -1 |
| `irq_pin` | i8 | ≥ -1 |
| `calibration` | object | `x_min,x_max,y_min,y_max`, `swapped_xy` |
| `sensitivity` | enum | `low`, `medium`, `high` |
| `swap_xy` | bool | |
| `invert_x` / `invert_y` | bool | |

### 2.2 UISettings (NVS)

| Field | Type | Validation |
|-------|------|------------|
| `brightness_pct` | u8 | 10–100 |
| `timeout_sec` | u16 | 30–600 (default 120) |
| `orientation` | enum | 0 / 90 / 180 / 270 |
| `temp_unit` | enum | `celsius`, `fahrenheit` |
| `language` | enum | `pt_br`, `en_us` |
| `touch_sensitivity` | enum | mirrors TouchConfig.sensitivity |
| `high_contrast` | bool | |
| `pin_lock_enabled` | bool | P2 |
| `pin_hash` | string | optional; P2 |

### 2.3 UIScreen (runtime FSM)

| Field | Type | Notes |
|-------|------|-------|
| `current` | enum | see contracts/ui-screens.md |
| `previous` | enum | back navigation |
| `params` | object | e.g. selected `profile_id`, dialog callback |

**P1 screens**: `home`, `start_material`, `start_custom`, `dialog_confirm`, `monitoring`, `dialog_message`  
**P2 screens**: settings_*, history_*, dialog_keypad, etc.

### 2.4 FilamentProfile (reuse 001)

Canonical entity in ConfigManager/ProfileManager. UI **MUST NOT** duplicate preset tables.

| id | name_pt (typical) | target_temp_c | default_duration_min | target_humidity_pct |
|----|--------------------|---------------|----------------------|---------------------|
| pla | PLA | 50 | 240 | 15 |
| petg | PETG | 65 | 240 | 15 |
| abs | ABS | 80 | 120 | 10 |
| tpu | TPU | 45 | 240 | 20 |
| nylon | Nylon | 70 | 360 | 10 |
| custom | Personalizado | user | user | user |

### 2.5 CycleRecord (on-device history, P2)

Circular buffer ≤50 in LittleFS/NVS. Fields per spec Key Entities. Prefer deriving summary from existing drying logs when possible to avoid dual writers.

### 2.6 Status broadcast extensions

`status/update` payload additions:

| Field | Type | Values |
|-------|------|--------|
| `ui_source` | string | `touch` \| `websocket` \| `auto` |
| `status` | string | includes **`paused`** |
| `paused` | bool | true when `SystemState::PAUSED` |

---

## 3. Validation rules (touch-originated)

| Action | Rules |
|--------|-------|
| Start (profile) | Profile exists; temp/humidity/duration within ProfileManager ranges; state READY (or STOPPED→READY) |
| Start (custom) | Same ranges; SafetyEngine hard limit still applies |
| Pause | State == DRYING; heater cutoff ≤500ms |
| Resume | State == PAUSED; SafetyEngine healthy |
| Stop | State ∈ {DRYING, PAUSED, COOLDOWN}; confirm dialog required on UI |

---

## 4. Relationships

```text
UISettings ──uses──► TouchConfig
TouchUiController ──reads──► ProfileManager (FilamentProfile[])
TouchUiController ──commands──► StateMachine (DryingSession)
StateMachine ──observed by──► Touch UI screens + WebSocketServer
```
