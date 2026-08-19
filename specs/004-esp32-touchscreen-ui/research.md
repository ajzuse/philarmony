# Research: ESP32 Touchscreen Interface

**Feature**: `004-esp32-touchscreen-ui` | **Date**: 2026-08-19  
**User plan focus**: Start drying from on-device filament presets; stop **or pause** without app.

## R1 — GUI framework: LVGL over LovyanGFX

**Decision**: Use **LVGL v8.3+** for screens/widgets; render via **LovyanGFX flush callback** on existing display drivers (ILI9341, ST7789, ILI9488, …). Keep headless/non-touch displays on the current `DisplayManager` path; when touch UI is enabled and panel ≥240×240, UI task owns the panel.

**Rationale**: Spec/README already target LVGL; LovyanGFX is already in `platformio.ini` with DMA SPI; combining both is a common ESP32 CYD pattern and preserves DEC-003 display matrix.

**Alternatives considered**:
- LovyanGFX-only custom UI — less structure for multi-screen/i18n
- SquareLine-generated only — OK later; not required for MVP
- TFT_eSPI + LVGL — would duplicate LovyanGFX stack already shipping in 001

## R2 — Touch input stack

**Decision**: Introduce `ITouchDriver` + `TouchManager` with auto-probe (I2C then SPI). Primary boards: **ESP32-2432S028 CYD → XPT2046**; capacitive FT6236/GT911/CST816S when configured. Map calibrated coords into LVGL `indev`. Debounce: 50ms press / 100ms release / 8px drag threshold. Calibration 4-point → NVS.

**Rationale**: Matches FR-001; keeps hardware polymorphism (DEC-002).

**Alternatives considered**: Hardcode CYD only — rejects multi-board constitution; poll-only without IRQ — OK fallback but IRQ preferred when pin present.

## R3 — Control bridge (touch ≡ WebSocket)

**Decision**: `TouchUiController` issues the **same** operations as `WebSocketServer` handlers: `ProfileManager::buildSessionFromRequest` / `listProfiles`, `StateMachine::startDrying`, `stopDrying`, **new** `pauseDrying` / `resumeDrying`. After each command, broadcast `status/update` with `ui_source: "touch"`. Last-write-wins with WS clients.

**Rationale**: Single source of truth; avoids divergent validation; SafetyEngine remains gate.

**Alternatives considered**: Separate touch-only control path — dual validation risk; direct actuator writes from UI — constitution failsafe violation.

## R4 — Pause / Resume (elevated to MVP)

**Decision**: Add `SystemState::PAUSED` between DRYING↔PAUSED. On pause: heater **off** immediately; exhaust fan **off** (MVP); **freeze** `elapsed_sec` accrual; keep session targets/profile. Resume → DRYING, restore control loop. Stop from PAUSED allowed (confirm). **Max pause: 30 minutes** → auto-stop with `pause_timeout`. Expose WS `control/pause` and `control/resume`. Feature **003 remains Stop-only** until it adopts these topics (DEC-012 unchanged).

**Rationale**: Explicit user plan request + clarify 2026-08-19 (Option B).

**Alternatives considered**: Unlimited pause — rejected (clarify); 60/120 min caps — not chosen.

## R4b — Power-loss resume

**Decision**: Persist interrupted drying/paused session; on boot **auto-resume** without prompt (SafetyEngine still gates heat). Clarify 2026-08-19 Option A.

**Alternatives considered**: Prompt Continue/Discard; never resume — rejected.

## R5 — Filament presets on Start flow

**Decision**: Start screen lists `ProfileManager::listProfiles()` (built-ins PLA/PETG/ABS/TPU/Nylon + custom). Tap preset → Confirm → `startDrying` with that `profile_id`. Custom path: numeric adjust temp/humidity/duration within ProfileManager validation ranges (30–80°C, 5–50%RH, 1–1440min).

**Rationale**: User plan focus + FR-004; reuses 001 profile storage (no duplicate MaterialPreset store).

**Alternatives considered**: Hardcoded UI presets diverging from NVS profiles — sync bugs with app.

## R6 — Screen set & v0.1 cut (clarify amend)

**Decision**: **Full v0.1 acceptance** — Start/Pause/Stop + mid-cycle adjust + Settings + History all required (clarify Option A). History: store 50, list newest 10 + Mais (Option C). Mid-cycle targets broadcast to WS (Option A). PIN lock remains optional polish within Settings/Advanced if time permits but not a separate deferral of core Settings/History.

**Rationale**: Clarify 2026-08-19 superseded earlier P1/P2 research split.

## R7 — Memory & tasking

**Decision**: Dedicated FreeRTOS **UI task** (Core 1, priority below control, above idle). LVGL draw buffers: partial buffer mode (e.g. 10–20 lines × width × 2) to stay under **&lt;200KB** UI RAM. Fonts: subset Noto Sans / Roboto via `lv_font` converter, embedded or LittleFS.

**Alternatives considered**: Full frame double-buffer RGB565 320×240 — ~300KB alone, exceeds budget on non-PSRAM boards.

## R8 — i18n & theme

**Decision**: String tables `ui_strings_pt_br` / `ui_strings_en_us`; runtime switch from UISettings. Dark high-contrast default theme (workshop lighting).

## R9 — Testing placement

**Decision**: Automated UI FSM + pause/start/stop flows on **004**. On-device smoke **VS-UI-1** (and follow-ons) only under `specs/005-manual-validation`.

## R10 — Board matrix for bring-up

**Decision**: Primary bring-up = **`env:esp32_2432s028`**. Secondary = LilyGo/ST7789 + external touch when available. Non-touch boards keep legacy `displayTask` status view.

---

## Resolved Technical Context items

| Item | Resolution |
|-------|------------|
| Language | C++17 / Arduino-ESP32 (001) |
| GUI | LVGL v8.3+ + LovyanGFX flush |
| Storage | NVS touch/UI + LittleFS fonts/history |
| Testing | Unity/native on 004; manuals in 005 |
| Pause | Firmware `PAUSED` + WS topics |
| Profiles | Reuse `FilamentProfile` / ProfileManager |

**All NEEDS CLARIFICATION cleared.**
