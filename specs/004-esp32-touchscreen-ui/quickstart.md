# Quickstart: ESP32 Touchscreen UI Validation

**Feature**: `004-esp32-touchscreen-ui`  
**Date**: 2026-08-19  
**Contracts**: `contracts/ui-screens.md`, `contracts/control-bridge.md`, `contracts/pause-resume-api.md`, `contracts/touch-input.md`

## Prerequisites

- PlatformIO + root `Makefile`
- Board with TFT + touch (preferred: **ESP32-2432S028 CYD**)
- Firmware base from `001` building (`make build ENV=esp32_2432s028`)
- Optional: WS client to observe `ui_source` / `paused`

## Setup

```bash
# From repo root
make build ENV=esp32_2432s028
make flash ENV=esp32_2432s028
make monitor ENV=esp32_2432s028   # optional
```

Host automated tests (once implemented):

```bash
make test
# or targeted native/UI flow suite when added under test/
```

## Validation scenarios (P1)

### QS-1 — Start from filament preset

1. Boot → Home interactive within 3s  
2. Tap **Iniciar Secagem** → preset list shows ProfileManager profiles (PLA, PETG, …)  
3. Select **PLA** → Confirmar  
4. **Expect**: Monitoring screen; heater/control active; WS `status` = `drying`, `ui_source` = `touch` when started from panel  

### QS-2 — Pause then resume

1. From active Monitoring, tap **Pausar**  
2. **Expect**: heater off ≤500ms; status `paused`; elapsed frozen  
3. Tap **Retomar**  
4. **Expect**: returns to `drying`; control resumes with same targets  

### QS-3 — Stop with confirm

1. From DRYING or PAUSED, tap **Parar** → confirm  
2. **Expect**: actuators safe; session stop reason user; UI returns Home/idle  

### QS-4 — WS last-write-wins (smoke)

1. Start from touch  
2. Send `control/stop` or `control/pause` from WS  
3. **Expect**: UI reflects new state without reboot  

## Manual catalog

On-device UI smoke sign-off: **`specs/005-manual-validation`** task **VS-UI-1** (T013). Do not leave open manual tasks on this feature’s `tasks.md`.

## Expected outcomes checklist

| ID | Pass criteria |
|-----|----------------|
| QS-1 | &lt;3 taps Home→Preset→Confirm; drying starts |
| QS-2 | Pause/resume match `pause-resume-api.md` |
| QS-3 | Stop confirm; safe actuators |
| QS-4 | Touch and WS stay coherent |
