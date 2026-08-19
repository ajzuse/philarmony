# Tasks: ESP32 Touchscreen Interface

**Input**: Design documents from `/specs/004-esp32-touchscreen-ui/`  
**Prerequisites**: `plan.md`, `spec.md` (Clarifications Session 2026-08-19), `research.md`, `data-model.md`, `contracts/`, `quickstart.md`  
**Stack**: C++17 / PlatformIO + LVGL v8.3+ + LovyanGFX + FreeRTOS (DEC-013)  
**Depends on**: Firmware `001` (`StateMachine`, `ProfileManager`, `DisplayManager`, WS)

**Locks (clarify)**: Full v0.1 acceptance · power-loss auto-resume · pause max 30 min · history 50 / list 10+Mais · mid-cycle targets WS broadcast

**Tests**: Automated Unity/native + flow tests on this feature. Open **manual** on-device smoke → `specs/005-manual-validation` as **VS-UI-*** only — no open `[MANUAL]` here.

## Format: `- [ ] [ID] [P?] [Story?] Description with file path`

- **[P]**: Parallelizable  
- **[Story]**: [US1]…[US6]

## Path Conventions

- Firmware: `src/`, `platformio.ini`, `test/`
- Feature docs: `specs/004-esp32-touchscreen-ui/`
- Manual catalog: `specs/005-manual-validation/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: LVGL/touch deps, dirs, workspace wiring

- [x] T001 Add `lvgl/lvgl@^8.3` (and touch-related deps as needed) to `platformio.ini` firmware `lib_deps` for `esp32devkitc`, `lilygo_tdisplay_v1`, and `esp32_2432s028`
- [x] T002 [P] Create `src/ui/`, `src/ui/screens/`, `src/ui/theme/`, `src/ui/assets/`, `src/drivers/touch/` directories with GPLv3 stub `.gitkeep` or README
- [x] T003 [P] Document LVGL + touch UI toolchain notes in `.vscode/workspace.json`
- [x] T004 [P] Add LVGL build flags / partial-buffer config notes in `platformio.ini` `build_flags` (RAM &lt;200KB UI budget)
- [x] T005 [P] Extend native `test` `build_src_filter` in `platformio.ini` for upcoming StateMachine/UI FSM host tests when files exist

**Checkpoint**: `pio pkg install` / project resolves LVGL for `ENV=esp32_2432s028`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Touch stack, LVGL port, PAUSED state, UI bridge, config — blocks all stories

**⚠️ CRITICAL**: No user story work until this phase completes

- [x] T006 Extend `ITouchDriver` + `TouchPoint` in `src/drivers/interfaces/IDriverInterfaces.hpp` per `contracts/touch-input.md`
- [x] T007 [P] Implement `TouchManager` probe/calibration API in `src/drivers/touch/TouchManager.hpp` and `src/drivers/touch/TouchManager.cpp`
- [x] T008 [P] Implement XPT2046 driver in `src/drivers/touch/Xpt2046Touch.hpp` and `src/drivers/touch/Xpt2046Touch.cpp` (CYD primary)
- [x] T009 [P] Implement FT6236 driver stubs/probe in `src/drivers/touch/Ft6236Touch.hpp` and `src/drivers/touch/Ft6236Touch.cpp`
- [x] T010 [P] Implement GT911 driver stubs/probe in `src/drivers/touch/Gt911Touch.hpp` and `src/drivers/touch/Gt911Touch.cpp`
- [x] T011 [P] Implement CST816S driver stubs/probe in `src/drivers/touch/Cst816sTouch.hpp` and `src/drivers/touch/Cst816sTouch.cpp`
- [x] T012 Add `TouchConfig` + `UISettings` fields and NVS load/save in `src/core/ConfigManager.hpp` and `src/core/ConfigManager.cpp`
- [x] T013 Add `SystemState::PAUSED`, transition matrix, `pauseDrying()` / `resumeDrying()`, 30-min pause timeout → `pause_timeout` in `src/core/StateMachine.hpp` and `src/core/StateMachine.cpp`
- [x] T014 Persist interrupted drying/paused session + boot **auto-resume** path in `src/core/StateMachine.cpp` and `src/main.cpp` (SafetyEngine still gates heat)
- [x] T015 Implement LVGL display flush + indev read port in `src/ui/LvglPort.hpp` and `src/ui/LvglPort.cpp` (LovyanGFX flush)
- [x] T016 [P] Add PT-BR/EN-US string tables in `src/ui/assets/ui_strings_pt_br.hpp` and `src/ui/assets/ui_strings_en_us.hpp`
- [x] T017 [P] Add dark high-contrast theme helpers in `src/ui/theme/ui_theme.hpp` and `src/ui/theme/ui_theme.cpp`
- [x] T018 Implement `TouchUiController` command bridge in `src/ui/TouchUiController.hpp` and `src/ui/TouchUiController.cpp` per `contracts/control-bridge.md`
- [x] T019 Wire FreeRTOS UI task + init (touch → LVGL → Home stub) in `src/main.cpp`; keep legacy `displayTask` for non-touch boards
- [x] T020 Add `control/pause` and `control/resume` handlers + `status`/`paused`/`ui_source` fields in `src/network/WebSocketServer.cpp` and `src/network/WebSocketServer.hpp` per `contracts/pause-resume-api.md`
- [x] T021 [P] Native/host unit tests for PAUSED transitions + 30-min timeout in `test/test_state_machine_pause/test_main.cpp` (or extend existing StateMachine native suite)
- [x] T022 [P] Native/host unit tests for touch debounce helpers in `test/test_touch_debounce/test_main.cpp`

**Checkpoint**: Firmware builds for `esp32_2432s028`; pause API + StateMachine host tests pass; UI task reaches Home stub with touch indev

---

## Phase 3: User Story 1 — Start Drying from Filament Presets (Priority: P1) 🎯 MVP

**Goal**: Home → presets from ProfileManager → Confirm → drying + Monitoring (Scenario 1 / FR-004 Start)

**Independent Test**: Idle device → Iniciar → select PLA → Confirmar → Monitoring shows drying; WS `status=drying`, `ui_source=touch` when started from panel (QS-1)

### Tests

- [x] T023 [P] [US1] Flow test start-from-profile (mock ProfileManager/StateMachine) in `test/test_ui_start_preset_flow/test_main.cpp`
- [x] T024 [P] [US1] Flow test custom start validation ranges in `test/test_ui_start_custom_flow/test_main.cpp`

### Implementation

- [x] T025 [P] [US1] Implement Home screen in `src/ui/screens/screen_home.hpp` and `src/ui/screens/screen_home.cpp`
- [x] T026 [P] [US1] Implement StartPresets screen listing `ProfileManager::listProfiles()` in `src/ui/screens/screen_start_presets.hpp` and `src/ui/screens/screen_start_presets.cpp`
- [x] T027 [P] [US1] Implement StartCustom screen (temp/humidity/duration steppers) in `src/ui/screens/screen_start_custom.hpp` and `src/ui/screens/screen_start_custom.cpp`
- [x] T028 [US1] Implement Confirm dialog + `TouchUiController` start via `buildSessionFromRequest` / `startDrying` in `src/ui/screens/screen_dialog_confirm.cpp` and `src/ui/TouchUiController.cpp`
- [x] T029 [US1] Implement Monitoring shell (live temp/humidity/targets/progress) in `src/ui/screens/screen_monitoring.hpp` and `src/ui/screens/screen_monitoring.cpp`
- [x] T030 [US1] Navigate Splash→Home→Start→Monitoring and set `ui_source=touch` on broadcast in `src/ui/` and `src/network/WebSocketServer.cpp`

**Checkpoint**: QS-1 pass on CYD or host flow suite green

---

## Phase 4: User Story 2 — Pause / Resume (Priority: P1)

**Goal**: Pausar/Retomar on Monitoring; 30-min auto-stop (Scenario 2b / FR-004 Pause)

**Independent Test**: During drying → Pausar → heater off, timer frozen → Retomar → drying; or wait/inject 30 min → `pause_timeout` (QS-2, QS-2b)

### Tests

- [x] T031 [P] [US2] Flow test pause/resume UI bridge in `test/test_ui_pause_resume_flow/test_main.cpp`
- [x] T032 [P] [US2] Flow test pause timeout auto-stop in `test/test_ui_pause_timeout_flow/test_main.cpp`

### Implementation

- [x] T033 [US2] Add Pausar/Retomar controls + paused visuals on Monitoring in `src/ui/screens/screen_monitoring.cpp`
- [x] T034 [US2] Wire `TouchUiController::pause` / `resume` to StateMachine + WS broadcast in `src/ui/TouchUiController.cpp`
- [x] T035 [US2] Show pause-timeout toast and return Home on `pause_timeout` in `src/ui/screens/screen_monitoring.cpp` and `src/ui/TouchUiController.cpp`

**Checkpoint**: QS-2 / QS-2b criteria met

---

## Phase 5: User Story 3 — Stop with Confirmation (Priority: P1)

**Goal**: Parar → confirm → safe stop from drying or paused (Scenario 2 / FR-004 Stop)

**Independent Test**: Parar → Confirm → actuators off ≤500ms; history/stop_reason user; Home (QS-3)

### Tests

- [x] T036 [P] [US3] Flow test stop confirm from drying and paused in `test/test_ui_stop_flow/test_main.cpp`

### Implementation

- [x] T037 [US3] Add Parar button + confirm dialog on Monitoring in `src/ui/screens/screen_monitoring.cpp` and `src/ui/screens/screen_dialog_confirm.cpp`
- [x] T038 [US3] Wire stop through `TouchUiController` → `stopDrying(USER_STOPPED)` / cooldown path + `ui_source=touch` in `src/ui/TouchUiController.cpp`

**Checkpoint**: QS-3 pass; stop allowed from PAUSED

---

## Phase 6: User Story 4 — Mid-Cycle Target Adjust + WS Broadcast (Priority: P1)

**Goal**: Tap targets on Monitoring → adjust → Aplicar → control loop + `status/update` (Scenario 3 / FR-007)

**Independent Test**: Change temp mid-cycle → loop uses new target within 1s; WS clients see targets + `ui_source=touch` (QS-5)

### Tests

- [x] T039 [P] [US4] Flow test mid-cycle target apply + broadcast fields in `test/test_ui_midcycle_targets_flow/test_main.cpp`

### Implementation

- [x] T040 [US4] Add target adjust overlay (± / keypad) in `src/ui/screens/screen_target_adjust.hpp` and `src/ui/screens/screen_target_adjust.cpp`
- [x] T041 [US4] Validate ranges and apply session targets via `TouchUiController` in `src/ui/TouchUiController.cpp`
- [x] T042 [US4] Broadcast updated targets on `status/update` with `ui_source=touch` in `src/network/WebSocketServer.cpp`

**Checkpoint**: QS-5 pass

---

## Phase 7: User Story 5 — On-Device Settings (Priority: P1)

**Goal**: Settings categories persist to NVS (Scenario 4 / FR-005)

**Independent Test**: Change brightness/unit/language → immediate UI effect; survives reboot

### Tests

- [x] T043 [P] [US5] Flow/unit tests for UISettings NVS round-trip in `test/test_ui_settings_nvs/test_main.cpp`

### Implementation

- [x] T044 [P] [US5] Settings main menu screen in `src/ui/screens/screen_settings_main.hpp` and `src/ui/screens/screen_settings_main.cpp`
- [x] T045 [P] [US5] WiFi status + Reconfigurar→hotspot trigger screen in `src/ui/screens/screen_settings_wifi.cpp`
- [x] T046 [P] [US5] Display brightness/timeout/orientation screen in `src/ui/screens/screen_settings_display.cpp`
- [x] T047 [P] [US5] Units + Language screens with live refresh in `src/ui/screens/screen_settings_units.cpp`
- [x] T048 [P] [US5] Touch sensitivity + 4-point calibration wizard in `src/ui/screens/screen_settings_touch.cpp`
- [x] T049 [US5] Sensors read-only + Advanced (device name, firmware, reset) in `src/ui/screens/screen_settings_advanced.cpp`
- [x] T050 [US5] Persist settings via ConfigManager and broadcast `config/*/ack` where applicable in `src/ui/TouchUiController.cpp` and `src/core/ConfigManager.cpp`

**Checkpoint**: Settings AC row pass; reboot persistence verified on device or NVS host test

---

## Phase 8: User Story 6 — On-Device History (Priority: P1)

**Goal**: Circular buffer 50; list newest 10 + Mais; detail sparklines (Scenario 5 / FR-006)

**Independent Test**: ≥1 completed cycle → Histórico shows newest 10; Mais loads older ≤50

### Tests

- [x] T051 [P] [US6] Unit/flow tests for CycleRecord ring + pagination in `test/test_ui_history_buffer/test_main.cpp`

### Implementation

- [x] T052 [US6] Implement CycleRecord store (50) in LittleFS/NVS helper `src/ui/history/CycleHistoryStore.hpp` and `src/ui/history/CycleHistoryStore.cpp`
- [x] T053 [US6] Append records on session end (incl. pause_timeout / user_stopped) from `src/ui/TouchUiController.cpp` / session callbacks
- [x] T054 [US6] History list screen (10 + Mais) in `src/ui/screens/screen_history_list.hpp` and `src/ui/screens/screen_history_list.cpp`
- [x] T055 [US6] History detail + sparkline rendering in `src/ui/screens/screen_history_detail.hpp` and `src/ui/screens/screen_history_detail.cpp`
- [x] T056 [P] [US6] Optional “Enviar para App” WS export hook stub in `src/ui/screens/screen_history_detail.cpp` (CSV/SD only if SD present)

**Checkpoint**: History AC row pass

---

## Phase 9: Polish & Cross-Cutting Concerns

**Purpose**: Sync, performance, manuals catalog, docs

- [x] T057 [P] WS last-write-wins visual feedback (toast) when remote command changes state in `src/ui/screens/screen_monitoring.cpp` (QS-4)
- [x] T058 Power-loss auto-resume UI path: boot → Monitoring drying/paused without prompt in `src/main.cpp` and `src/ui/` (QS-6)
- [x] T059 [P] Screen timeout dim/restore backlight behavior in `src/ui/LvglPort.cpp` / `TouchUiController`
- [x] T060 [P] Expand `specs/005-manual-validation/tasks.md` VS-UI-* stages (VS-UI-1 start/pause/stop, settings, history, power-loss) — catalog only; no `[MANUAL]` on 004
- [x] T061 [P] Create on-device results ledger stub `specs/004-esp32-touchscreen-ui/checklists/quickstart-validation.md` for VS-UI sign-off recording
- [x] T062 [P] Update `specs/001-filament-dryer-esp32/contracts/websocket-api.md` (or cross-link) to reference `control/pause` / `control/resume` + `paused` status fields
- [x] T063 [P] Sync DEC-013 consequences if needed in `.specify/memory/catalog/entries/DEC-013-touchscreen-ui-pause.md`
- [x] T064 Run host `make test` including new UI/StateMachine suites; fix failures
- [x] T065 Verify `make build ENV=esp32_2432s028` succeeds with LVGL UI enabled
- [x] T066 [P] Update feature `quickstart.md` expected outcomes if implementation paths drifted

**Checkpoint**: v0.1 acceptance stories complete; manuals only open under 005

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundational (Phase 2)**: Depends on Setup — **BLOCKS** all user stories
- **US1–US6**: Depend on Foundational; prefer sequential US1→US3 before US4–US6 (Monitoring shell from US1)
- **Polish**: After desired stories complete

### User Story Dependencies

```text
Phase1 → Phase2 → US1 (Start/Monitoring shell)
                 → US2 (Pause) ── needs US1 Monitoring
                 → US3 (Stop)  ── needs US1 Monitoring
                 → US4 (Adjust)── needs US1 Monitoring
                 → US5 (Settings) ── after Phase2 (can parallel US2–US4 if staffed)
                 → US6 (History) ── needs session end hooks (US1–US3)
Polish → after US1–US6
```

### Parallel Opportunities

- T002–T005 setup in parallel
- T007–T011 touch drivers in parallel after T006
- T016–T017 theme/i18n parallel with T015
- T023–T024 US1 tests parallel; T025–T027 screens parallel after foundation
- US5 screens T044–T048 largely parallel
- Polish T057, T059–T063, T066 parallel

---

## Parallel Example: User Story 1

```bash
# Tests in parallel:
Task: "Flow test start-from-profile in test/test_ui_start_preset_flow/test_main.cpp"
Task: "Flow test custom start in test/test_ui_start_custom_flow/test_main.cpp"

# Screens in parallel after controller stubs exist:
Task: "Home screen in src/ui/screens/screen_home.cpp"
Task: "StartPresets in src/ui/screens/screen_start_presets.cpp"
Task: "StartCustom in src/ui/screens/screen_start_custom.cpp"
```

---

## Implementation Strategy

### MVP First (US1 only)

1. Phase 1 Setup  
2. Phase 2 Foundational (PAUSED + LVGL + bridge — needed even for start broadcast)  
3. Phase 3 US1 Start + Monitoring shell  
4. **STOP and VALIDATE** QS-1  

### Incremental Delivery (v0.1)

1. US2 Pause → US3 Stop → validate QS-2/2b/3  
2. US4 Mid-cycle → QS-5  
3. US5 Settings → US6 History  
4. Polish (QS-4/QS-6, 005 VS-UI catalog, build/test)  

### Suggested MVP Scope

**US1 only** (Start from presets + Monitoring) for first demo; v0.1 acceptance still requires US2–US6 before release sign-off.

---

## Notes

- No open `[MANUAL]` tasks on this file — use `005` VS-UI-*  
- App `003` remains Stop-only until it adopts pause topics  
- All tasks use checklist format with file paths  
- Constitution: GPLv3 headers, PT-BR/EN-US, SafetyEngine authority  

**Constitution compliance:** PROJECT_NAME = Philarmony, provisions per `.specify/memory/constitution.md`
