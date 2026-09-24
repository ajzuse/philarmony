# Tasks: Ambient Humidity Sensor Support

**Input**: Design documents from `/specs/006-ambient-humidity-sensor/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/ambient-sensor-api.md, quickstart.md
**Stack**: C++17 / PlatformIO + ESP-IDF v5.1+ (same as 001)
**Depends on**: Firmware 001 (SensorConfig, ISensorDriver, DriverRegistry, controlLoopTask, WebSocketServer)

**Locks**: Configurable margin 0–10%RH · 30s ambient timeout · status on touch UI + WebSocket · chamber-only backward compatible

**Tests**: Automated Unity/native flow tests on this product spec. Open manual validation → `specs/005-manual-validation`.

## Format: `- [x] [ID] [P?] [Story?] Description with file path`

- **[P]**: Parallelizable
- **[Story]**: [US1]…[US4]

## Path Conventions

- Firmware: `src/`, `platformio.ini`, `test/`
- Feature docs: `specs/006-ambient-humidity-sensor/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Extend SensorConfig and UISettings with ambient fields; no behavioral changes yet

- [x] T001 Add ambient sensor fields (`ambient_type`, `ambient_i2c_address`, `ambient_gpio_pin`, `ambient_sda_pin`, `ambient_scl_pin`, `ambient_temperature_offset/scale`, `ambient_humidity_offset/scale`) to `SensorConfig` struct in `src/core/ConfigManager.hpp`
- [x] T002 [P] Add `ambient_ventilation_margin_pct` field (float, default 2.0, range 0–10) to `UISettings` struct in `src/core/ConfigManager.hpp`
- [x] T003 Update `getSensorConfig()` / `setSensorConfig()` JSON serialization in `src/core/ConfigManager.cpp` to include/exclude ambient fields (backward compatible — missing fields default empty)
- [x] T004 [P] Update `getUiSettings()` / `setUiSettings()` JSON serialization in `src/core/ConfigManager.cpp` to include `ambient_ventilation_margin_pct`

**Checkpoint**: `SensorConfig` and `UISettings` persist ambient fields via NVS; existing single-sensor configs unaffected

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Config parser ambient role, sensor init, ambient-aware fan logic — blocks all user stories

**⚠️ CRITICAL**: No user story work until this phase completes

- [x] T005 Extend `HardwareConfigParser::parseSensors()` in `src/core/HardwareConfigParser.cpp` to recognize `"role": "ambient"` in sensor JSON objects; validate type against `DriverRegistry` catalog; detect pin conflicts with chamber sensor; reject unsupported types with error message per `contracts/ambient-sensor-api.md` §1
- [x] T006 Add `ISensorDriver* ambientSensor = nullptr` global pointer in `src/main.cpp`; create ambient sensor instance in `initializeSensors()` via `DriverRegistry::createSensor()` when `ambient_type` is non-empty; apply calibration via existing `applySensorCalibration()`
- [x] T007 Implement ambient-aware fan stop logic in `controlLoopTask()` in `src/main.cpp`: track `lastValidAmbientMs`, compute `ambientAvailable` (valid && <30s), override `fanPower = 0` when `ambientAvailable && chamberHumidity <= ambientHumidity + margin`; read margin from `UISettings`; fallback to chamber-only when `!ambientAvailable`
- [x] T008 Add ambient fields (`ambient_temp_c`, `ambient_humidity_pct`, `ambient_sensor_type`) to WebSocket `buildStatusPayload()` in `src/network/WebSocketServer.cpp`; omit when ambient not configured per `contracts/ambient-sensor-api.md` §4
- [x] T009 Add ambient telemetry fields to `controlLoopTask()` broadcast payload in `src/main.cpp` (ambient_temp_c, ambient_humidity_pct from calibrated ambient reading)

**Checkpoint**: Firmware builds; ambient sensor read + fan stop logic + WS broadcast functional; chamber-only builds unaffected

---

## Phase 3: User Story 1 — Ambient Sensor Improves Fan Stop Decision (Priority: P1) 🎯 MVP

**Goal**: Dual-sensor setup: fan stops when chamber humidity ≤ ambient + margin; fan resumes when chamber rises above ambient + margin

**Independent Test**: QS-2 + QS-3 from quickstart.md: configure dual sensors, start cycle, verify fan stops at margin threshold, verify margin adjustment works

### Tests

- [x] T010 [P] [US1] Flow test ambient fan stop logic in `test/test_ambient_fan_stop/test_main.cpp`: validate fanPower=0 when chamberHumidity ≤ ambientHumidity + margin, fanPower=duty when above, fallback when ambient invalid

### Implementation

- [x] T011 [US1] Implement ambient-aware fan resume: when chamber humidity rises above ambient + margin during active cycle, restore fan duty in `src/main.cpp` (part of T007 logic, ensure bidirectional)

**Checkpoint**: QS-2/QS-3 pass; fan stops and resumes based on ambient comparison

---

## Phase 4: User Story 2 — Single Chamber Sensor Backward Compatible (Priority: P1)

**Goal**: Chamber-only builds run identically to pre-006 firmware

**Independent Test**: QS-1 from quickstart.md: no ambient configured, verify identical behavior

### Tests

- [x] T012 [P] [US2] Flow test chamber-only backward compatibility in `test/test_ambient_chamber_only/test_main.cpp`: validate no ambient fields in status, fan behavior unchanged, no ambient pointer created

### Implementation

- [x] T013 [US2] Verify and document that empty `ambient_type` (default) produces zero ambient overhead: no sensor created, no telemetry fields, no fan logic branch taken in `src/main.cpp`

**Checkpoint**: QS-1 pass; no regressions on single-sensor builds

---

## Phase 5: User Story 3 — Configure Ambient Sensor From Shared Catalog (Priority: P2)

**Goal**: Any supported sensor type works as ambient; mixed types validated; config persists

**Independent Test**: QS-5 + QS-6 from quickstart.md: mixed sensor types, config save/reload, status fields present

### Tests

- [x] T014 [P] [US3] Flow test config parser ambient role in `test/test_ambient_config_parser/test_main.cpp`: validate `"role": "ambient"` parsed correctly, pin conflicts rejected, unsupported types rejected, backward compat (no role = chamber)
- [x] T015 [P] [US3] Flow test NVS round-trip in `test/test_ambient_nvs_roundtrip/test_main.cpp`: validate ambient fields persist/load via ConfigManager, defaults correct when ambient not configured

### Implementation

- [x] T016 [US3] Add ambient sensor info to `screen_settings_sensors.cpp` in `src/ui/screens/`: display ambient type, I2C address, pins when configured; hide when not

**Checkpoint**: QS-5/QS-6 pass; config persistence verified

---

## Phase 6: User Story 4 — Ambient Sensor Fault Falls Back Safely (Priority: P2)

**Goal**: Ambient failure → 30s fallback → chamber-only; no device fault; recovery on reconnect

**Independent Test**: QS-4 from quickstart.md: disconnect ambient mid-cycle, verify fallback within 30s, verify recovery

### Tests

- [x] T017 [P] [US4] Flow test ambient timeout fallback in `test/test_ambient_timeout_fallback/test_main.cpp`: validate 30s timeout triggers fallback, ambientAvailable flips false, fan resumes chamber-only duty, recovery on valid reading

### Implementation

- [x] T018 [US4] Implement ambient status display fallback in `src/ui/screens/screen_monitoring.cpp`: show "--" / "N/A" for ambient readings when `!ambientAvailable`; restore values on recovery
- [x] T019 [US4] Implement WebSocket null fields on fallback: send `ambient_temp_c: null, ambient_humidity_pct: null` in `src/network/WebSocketServer.cpp` when ambient unavailable

**Checkpoint**: QS-4 pass; fault fallback + recovery verified

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Touch UI ambient display, DisplayManager layout, margin settings UI

- [x] T020 [P] Add ambient readings (temp + humidity labels) to monitoring screen below chamber values in `src/ui/screens/screen_monitoring.cpp` when ambient configured
- [x] T021 [P] Add `ambient_temp_c` and `ambient_humidity_pct` to `all_fields` in `autoConfigureLayout()` in `src/drivers/display/DisplayManager.cpp` for larger displays
- [x] T022 [P] Add ambient ventilation margin setting (0–10%RH stepper) to touch UI settings in `src/ui/UiApp.cpp` or new settings screen; persist via `UISettings`
- [x] T023 Add `live_ambient_temp_c_` / `live_ambient_humidity_pct_` fields to `UiApp` in `src/ui/UiApp.hpp`; update `setLiveReadings()` to accept ambient values; pass from `controlLoopTask()` in `src/main.cpp`
- [x] T024 Run host `make test` including new ambient test suites; fix failures
- [x] T025 Verify `make build ENV=esp32_2432s028` succeeds with ambient sensor code

**Checkpoint**: v0.1 acceptance complete; touch UI + WS + display all expose ambient data

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundational (Phase 2)**: Depends on Setup — **BLOCKS** all user stories
- **US1–US2**: Depend on Foundational; US1 and US2 can run in parallel
- **US3–US4**: Depend on Foundational; can run parallel with US1/US2
- **Polish**: After US1–US4 complete

### User Story Dependencies

```text
Phase1 → Phase2 → US1 (fan stop logic) ── parallel with US2
                  → US2 (backward compat) ── parallel with US1
                  → US3 (config + status) ── parallel with US1/US2
                  → US4 (fault fallback) ── parallel with US1/US2/US3
Polish → after US1–US4
```

### Parallel Opportunities

- T001–T002 setup in parallel
- T010, T012, T014, T015, T017 all test suites in parallel (different files)
- T020–T021 polish screens in parallel
- US1 and US2 are fully independent after Phase 2

---

## Implementation Strategy

### MVP First (US1 only)

1. Phase 1 Setup
2. Phase 2 Foundational (ambient sensor init + fan logic — needed for US1)
3. Phase 3 US1 (fan stop + resume)
4. **STOP and VALIDATE** QS-2/QS-3

### Incremental Delivery (v0.1)

1. US2 backward compat → QS-1
2. US3 config + status → QS-5/QS-6
3. US4 fault fallback → QS-4
4. Polish (touch UI, display, margin settings)

### Suggested MVP Scope

**US1 only** (ambient fan stop) for first demo; v0.1 acceptance requires US2–US4 before release sign-off.

---

## Notes

- No open `[MANUAL]` tasks on this file — use `005` VS-UI-*
- All tasks use checklist format with file paths
- Constitution: GPLv3 headers, PT-BR/EN-US, SafetyEngine authority unchanged
- Ambient sensor reuses existing ISensorDriver — no new driver code

**Constitution compliance:** PROJECT_NAME = Philarmony, provisions per `.specify/memory/constitution.md`

---

## Phase 8: Convergence

- [x] T026 Apply ambient calibration fields (`ambient_temperature_offset/scale`, `ambient_humidity_offset/scale`) to ambient readings in `src/main.cpp` instead of chamber calibration per FR-001 (contradicts)
- [x] T027 Reject ambient sensor pin / I2C-address conflicts with the chamber sensor at parse time in `HardwareConfigParser::checkPinConflicts()` (`src/core/HardwareConfigParser.cpp`) per contract §1 (missing)
- [x] T028 Add ambient ventilation margin stepper (0–10%RH) to `src/ui/screens/screen_settings_sensors.cpp` with handler in `src/ui/UiApp.cpp` persisting via `UISettings` per FR-004 (partial)
- [x] T029 Clamp `ambient_ventilation_margin_pct` to 0–10 on load/use in `src/core/ConfigManager.cpp` and `src/main.cpp` per data-model §3 (partial)
- [x] T030 Show ambient row as `"--"`/`"N/A"` in `src/ui/screens/screen_monitoring.cpp` when ambient is configured but unavailable per contract §7 (partial)
- [x] T031 Extract ambient fan-stop decision into a testable unit and cover it from `test/test_ambient_fan_stop/test_main.cpp` and `test/test_ambient_timeout_fallback/test_main.cpp` per research R10 (partial)
- [x] T032 Add pin-conflict and unsupported-type assertions to `test/test_ambient_config_parser/test_main.cpp` per T014 (partial)

---

## Phase 9: Convergence

- [x] T033 Add `ambient_temp_c` / `ambient_humidity_pct` (null on fallback) to the COOLDOWN `status/update` broadcast in `src/main.cpp` per FR-006 (partial)

---

## Phase 10: Convergence

- [x] T034 CRITICAL Register the 006 ambient-humidity-sensor research and decisions in the shared memory catalog (new `.specify/memory/catalog/entries/DEC-014-ambient-humidity-sensor.md` plus index row in `.specify/memory/catalog/README.md`) per Constitution: Memória Compartilhada de Pesquisa e Conhecimento (missing)
- [x] T035 Bind ambient sensor `bus` parsing to an isolated ambient-specific field instead of passing the chamber `config.i2c_bus` by reference in `HardwareConfigParser::parseSensors()` (`src/core/HardwareConfigParser.cpp`) per data-model §2.1 (partial)
- [x] T036 Validate/clamp ambient calibration fields (`ambient_temperature_offset/scale`, `ambient_humidity_offset/scale`) to the data-model §2.1 ranges on parse and load in `src/core/HardwareConfigParser.cpp` / `src/core/ConfigManager.cpp` per data-model §2.1 (partial)
