# DEC-013 — On-Device Touch UI (LVGL) + Pause State

**Tags:** touchscreen, lvgl, pause, ui, esp32  
**Feature:** `004-esp32-touchscreen-ui`  
**Source:** `specs/004-esp32-touchscreen-ui/research.md`  
**Updated:** 2026-08-19  
**Status:** Accepted

## Decision

1. Ship on-device touch UI with **LVGL v8.3+** flushed through existing **LovyanGFX** display drivers.
2. Add `ITouchDriver` / `TouchManager` with auto-probe (XPT2046, STMPE610, FT6236, GT911, CST816S).
3. Route all touch start/stop/pause through **ProfileManager + StateMachine** (same path as WebSocket).
4. Elevate **Pause/Resume to MVP**: new `SystemState::PAUSED`, WS topics `control/pause` / `control/resume`. Heater off; elapsed frozen; fan off (MVP).
5. Filament presets on Start = live `ProfileManager` list (no duplicate tables).
6. Control app **003 remains Stop-only** (DEC-012) until it adopts pause topics.
7. Automated flows on 004; manuals → `005` VS-UI-*.

## Rationale

User plan requires standalone start-from-presets and stop/pause. Firmware 001 had display telemetry only and no pause state; UI-only pause would desync safety and WS clients.

## Alternatives rejected

| Option | Why rejected |
|--------|----------------|
| LovyanGFX-only custom widgets | Higher cost for multi-screen/i18n vs LVGL |
| Pause as UI placeholder only | Misleading heater/timer behavior |
| Duplicate hard-coded presets in UI | Drift from NVS/app profiles |
| Force 003 Pause in same release | Out of scope; contract documented for later |

## Consequences

- Extend StateMachine transition matrix and status stream (`paused`)
- Add `src/ui/` + `src/drivers/touch/`; LVGL in `platformio.ini` + workspace.json
- Amend 001 WS contract consumers via `contracts/pause-resume-api.md`
- Update DEC-003 bring-up notes for CYD touch (optional follow-up)
