# Implementation Plan: ESP32 Touchscreen Interface

**Branch**: `004-esp32-touchscreen-ui` | **Date**: 2026-08-19 | **Spec**: `specs/004-esp32-touchscreen-ui/spec.md`

**Input**: Feature specification from `/specs/004-esp32-touchscreen-ui/spec.md`  
**User plan focus (2026-08-19)**: On-device start with filament presets; stop **or pause** drying without app/WS.

**Depends on**: Firmware `001-filament-dryer-esp32` (StateMachine, ProfileManager, DisplayManager, WebSocket, NVS). Manual smoke → `005-manual-validation` (VS-UI-*).

## Summary

Add a touch-capable on-device UI to the ESP32 filament dryer so operators can run a drying cycle standalone: pick a built-in/custom filament profile, start, monitor, **pause/resume** (30 min max then auto-stop), or stop with confirmation; adjust mid-cycle targets (broadcast to WS); configure device settings; view history (50 stored, list shows 10 + Mais). Touch commands reuse the same control path as WebSocket (`ProfileManager` + `StateMachine` + SafetyEngine). Stack: **LVGL v8+** flushed through existing **LovyanGFX** display drivers; new `ITouchDriver` + probe; FreeRTOS UI task. Pause requires `SystemState::PAUSED` and WS `control/pause` / `control/resume`. Power loss: **auto-resume** interrupted drying/paused session (no prompt). App 003 stays Stop-only until it opts in.

**Clarify session 2026-08-19** locked full v0.1 acceptance (Settings/History/mid-cycle required).

## Technical Context

**Language/Version**: C++17 / ESP-IDF v5.1+ (Arduino Core via PlatformIO) — same as 001

**Primary Dependencies**:
- Existing: FreeRTOS, LovyanGFX, ArduinoJson, Preferences/NVS, LittleFS, ESPAsyncWebServer
- New: **LVGL v8.3+** (PlatformIO `lvgl/lvgl`) with LovyanGFX flush/input ports
- Touch controllers via LovyanGFX / dedicated drivers: XPT2046, STMPE610, FT6236, GT911, CST816S

**Storage**:
- NVS: `TouchConfig`, `UISettings`, calibration matrix, optional PIN hash
- LittleFS/SPIFFS: font subsets + UI string tables (PT-BR/EN-US); cycle history ring (last 50) if not already in 001 logs

**Testing**:
- Automated: Unity native + host flow tests for UI state machine, profile start validation, pause/stop transitions, touch debounce (product spec 004)
- Manual on-device smoke: `005-manual-validation` **VS-UI-1** (and related VS-UI-* as added)

**Target Platform**: ESP32 / ESP32-S3 with color TFT ≥240×240 + touch (primary: ESP32-2432S028 CYD ILI9341+XPT2046; also ST7789/ILI9488 + capacitive where wired)

**Project Type**: Embedded firmware UI extension (same firmware binary as 001)

**Performance Goals**:
- Touch press → visual feedback &lt;50ms
- UI frame rate ≥30fps (60fps target on SPI DMA displays)
- Boot → interactive Home &lt;3s (touch init included)
- UI RAM overhead &lt;200KB; flash assets ≤500KB
- Shared control path: Start/Stop/Pause applied to StateMachine within 1 control tick (&lt;100ms)

**Constraints**:
- SafetyEngine remains authoritative; touch cannot bypass temp/power limits
- Non-blocking: LVGL tick + flush on UI task; no blocking in control loop
- Last-write-wins with WebSocket; `status/update.ui_source` = `touch` | `websocket` | `auto`
- PT-BR primary / EN-US secondary strings externalized
- Open manuals only in `005`; automation on this feature
- GPLv3 headers on new sources; new deps in `platformio.ini` + `.vscode/workspace.json`

**Scale/Scope**:
- **v0.1 acceptance (clarify 2026-08-19):** Home, Start (presets + custom), Monitoring, Pause/Resume (30 min max), Stop+confirm, mid-cycle target adjust (WS broadcast), Settings (all categories), History (50 stored / list 10 + Mais), power-loss **auto-resume**
- Out of scope: multi-touch, full soft keyboard, OTA from UI, remote mirror

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Orientação a Objetos e Segurança de Hardware**: `ITouchDriver`, `TouchUiController`, screen state objects; start/pause/stop go through validated ProfileManager/StateMachine/SafetyEngine.
- [x] **Desempenho Máximo e Eficiência**: Static LVGL buffers where possible; UI task budgeted; flush via existing DMA paths; no dynamic alloc in control path.
- [x] **Failsafe e Proteção de Hardware**: Pause cuts heater; Stop/fault still force safe actuators; confirmation on Stop; SafetyEngine unchanged authority.
- [x] **Workspace de Dependências**: LVGL + touch libs added to `platformio.ini` and `.vscode/workspace.json`.
- [x] **Teste Automatizado e Qualidade**: Flow tests for start-preset / pause / stop on 004; manuals → `005` VS-UI-*.
- [x] **Documentação Sincronizada**: README via `after_plan` hook.
- [x] **Memória Compartilhada**: DEC-013 (touch UI + pause) in catalog.
- [x] **Revisão e Aprovação Explícita de Commit / Cavemen / Languages**: unchanged process rules.

**Post-design**: Gates OK. Justified complexity: LVGL + pause state (see Complexity Tracking).

## Project Structure

### Documentation (this feature)

```text
specs/004-esp32-touchscreen-ui/
├── plan.md              # This file
├── research.md          # Phase 0
├── data-model.md        # Phase 1
├── quickstart.md        # Phase 1 validation guide
├── contracts/
│   ├── touch-input.md
│   ├── ui-screens.md
│   ├── control-bridge.md
│   └── pause-resume-api.md
└── tasks.md             # Phase 2 (/speckit.tasks — not created here)
```

### Source Code (repository root)

```text
platformio.ini                    # + lvgl, touch-related deps
src/
├── main.cpp                      # Create UI task; wire TouchUiController
├── core/
│   ├── StateMachine.hpp/.cpp    # + SystemState::PAUSED; pause/resume APIs
│   ├── ProfileManager.*          # Unchanged start-from-profile path (reuse)
│   └── ConfigManager.hpp        # + TouchConfig, UISettings fields
├── drivers/
│   ├── interfaces/
│   │   └── IDriverInterfaces.hpp # + ITouchDriver
│   ├── touch/
│   │   ├── TouchManager.hpp/.cpp
│   │   ├── Xpt2046Touch.*
│   │   ├── Ft6236Touch.*
│   │   ├── Gt911Touch.*
│   │   └── Cst816sTouch.*       # + STMPE610 as needed
│   └── display/                  # LovyanGFX flush hooks for LVGL
└── ui/
    ├── TouchUiController.hpp/.cpp  # Command bridge → StateMachine/WS broadcast
    ├── LvglPort.hpp/.cpp           # Display flush + indev read
    ├── screens/                    # Home, StartPresets, Monitoring, Dialogs, …
    ├── assets/                    # Fonts / i18n tables
    └── theme/                      # Dark high-contrast theme
test/
└── (native/host flows for UI FSM + pause transitions)
```

**Structure Decision**: Extend the single firmware tree from 001. New `src/ui/` for LVGL screens and bridge; new `src/drivers/touch/` for touch drivers. No separate app binary.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| LVGL dependency (+RAM/flash) | Spec requires multi-screen touch UI, i18n, 30fps; LovyanGFX alone would reinvent widgets | Custom immediate-mode UI — higher maintenance, weaker a11y/i18n |
| `PAUSED` state + WS pause/resume | User plan requires pause on-device; 001 has only start/stop; 003 Stay Stop-only until documented | UI-only pause without firmware state — desync with WS/app and unsafe heater handling |

**Constitution compliance:** PROJECT_NAME = Philarmony, provisions per `.specify/memory/constitution.md`
