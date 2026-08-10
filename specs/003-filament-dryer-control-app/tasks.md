# Tasks: Filament Dryer Control App

**Input**: Design documents from `/specs/003-filament-dryer-control-app/`  
**Prerequisites**: `plan.md` (amended 2026-08-10), `spec.md` (Clarifications Session), `research.md`, `data-model.md`, `contracts/`, `quickstart.md`  
**Stack**: Flutter (FVM) + `philarmony_core` + WS `001` + Drift + Riverpod + go_router + `pdf`/`printing` (DEC-010/012)

**Locks (clarify)**: WS=`001` only · registry=`KnownDevice` · export=CSV+PDF MVP · background WS required iOS+Android · Stop-only (no Pause)

**Tests**: Automated widget/flow/CI on this feature. Open **manual** smoke (quickstart VS-*) → `specs/005-manual-validation` only — no open `[MANUAL]` here.

## Format: `- [ ] [ID] [P?] [Story?] Description with file path`

- **[P]**: Parallelizable  
- **[Story]**: [US1]…[US7]

## Path Conventions

- Shared: `packages/philarmony_core/`
- App: `apps/filament-dryer-control/`
- Tooling: root `Makefile`, `.fvmrc`, `.github/workflows/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Single Flutter app (all platforms) + monorepo wiring

- [ ] T001 Create Flutter app skeleton in `apps/filament-dryer-control/pubspec.yaml` depending on `packages/philarmony_core` (path)
- [ ] T002 [P] Enable platforms android/ios/windows/macos/linux under `apps/filament-dryer-control/`
- [ ] T003 [P] Add module dirs `lib/{app,shell,platform,data,device,features,l10n,theme}/` per `contracts/package-structure.md`
- [ ] T004 [P] Declare deps in `apps/filament-dryer-control/pubspec.yaml`: `web_socket_channel`, `multicast_dns`, `flutter_riverpod`, `go_router`, `fl_chart`, `drift`, `drift_flutter`, `sqlite3_flutter_libs`, `flutter_local_notifications`, `shared_preferences`, `flutter_localizations`, `pdf`, `printing` (or equivalent PDF stack)
- [ ] T005 [P] Extend `.gitignore` for control-app `build/`, `.dart_tool/`, `dist/` if missing
- [ ] T006 [P] Document control-app run targets in `.vscode/workspace.json`
- [ ] T007 Add Make targets `run-control`, `test-control`, `package-control-linux` in root `Makefile`
- [ ] T008 [P] Add GPLv3 stubs + overview in `apps/filament-dryer-control/README.md` (WS port 80, entitlements note)

**Checkpoint**: `flutter pub get` + `flutter analyze` OK in control app

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core DTOs, WS client, DB, adaptive shell — blocks all stories

**⚠️ CRITICAL**: No user story work until this phase completes

- [ ] T009 [P] Add WS envelope + `StatusSnapshot` + `FaultEvent` in `packages/philarmony_core/lib/src/models/ws_models.dart`
- [ ] T010 [P] Add `KnownDevice` model in `packages/philarmony_core/lib/src/models/known_device.dart` (not installer `DeviceProfile`)
- [ ] T011 [P] Add `StartCycleRequest` / `StopCycleRequest` + validator in `packages/philarmony_core/lib/src/validation/cycle_command_validator.dart`
- [ ] T012 [P] Add WS codecs matching `specs/001-filament-dryer-esp32/contracts/websocket-api.md` in `packages/philarmony_core/lib/src/ws/ws_codec.dart` (`max_duration_min`, `config/hardware`)
- [ ] T013 Export new APIs from `packages/philarmony_core/lib/philarmony_core.dart`
- [ ] T014 Unit tests codecs + cycle validator in `packages/philarmony_core/test/ws_codec_test.dart` and `packages/philarmony_core/test/cycle_command_validator_test.dart`
- [ ] T015 Define `PhilarmonyWsClient` + `DeviceDiscovery` + `BackgroundSession` interfaces in `apps/filament-dryer-control/lib/device/device_interfaces.dart`
- [ ] T016 Implement reconnecting WS client (`ws://host/ws`, backoff ≤60s) in `apps/filament-dryer-control/lib/device/philarmony_ws_client.dart`
- [ ] T017 Implement `FakePhilarmonyWsClient` in `apps/filament-dryer-control/lib/device/fake_philarmony_ws_client.dart`
- [ ] T018 Drift DB (`KnownDevice`, `DryingCycle`, `CycleSample`, `PendingCommand`) in `apps/filament-dryer-control/lib/data/app_database.dart`
- [ ] T019 [P] App preferences in `apps/filament-dryer-control/lib/data/app_preferences.dart`
- [ ] T020 [P] `AppBreakpoint` + `AdaptiveScaffold` in `apps/filament-dryer-control/lib/shell/` per `contracts/adaptive-layout.md`
- [ ] T021 go_router shell + placeholders in `apps/filament-dryer-control/lib/app.dart` and `apps/filament-dryer-control/lib/shell/app_router.dart`
- [ ] T022 [P] ARB PT-BR/EN-US stubs in `apps/filament-dryer-control/lib/l10n/`
- [ ] T023 [P] Theme in `apps/filament-dryer-control/lib/theme/app_theme.dart`
- [ ] T024 Riverpod `ProviderScope` + session registry stub in `apps/filament-dryer-control/lib/device/session_providers.dart`
- [ ] T025 Smoke widget test adaptive shell in `apps/filament-dryer-control/test/shell/adaptive_scaffold_test.dart`

**Checkpoint**: `make test-core` + shell tests pass with fake WS

---

## Phase 3: User Story 1 — Device Discovery & Connection (Priority: P1) 🎯 MVP

**Goal**: mDNS / manual → WS connect; status; remember last `KnownDevice` (FR-001)

**Independent Test**: Manual host:80`/ws` connects via fake/real; Scan lists services; Connecting/Connected/Disconnected/Error; auto-reconnect

### Tests

- [ ] T026 [P] [US1] Flow test manual connect in `apps/filament-dryer-control/test/features/discover/connect_flow_test.dart`
- [ ] T027 [P] [US1] WS subscribe-on-connect test in `apps/filament-dryer-control/test/device/ws_client_test.dart`

### Implementation

- [ ] T028 [P] [US1] mDNS adapter `_philarmony._tcp` in `apps/filament-dryer-control/lib/platform/mdns_discovery.dart`
- [ ] T029 [P] [US1] `KnownDevice` repository in `apps/filament-dryer-control/lib/data/known_device_repository.dart`
- [ ] T030 [US1] Discover UI (scan + manual form) in `apps/filament-dryer-control/lib/features/discover/discover_page.dart`
- [ ] T031 [US1] Connection status chip in `apps/filament-dryer-control/lib/features/discover/connection_status.dart`
- [ ] T032 [US1] Last-connected + auto-connect in `apps/filament-dryer-control/lib/features/discover/discover_controller.dart`
- [ ] T033 [US1] iOS Local Network / Android Wi‑Fi permissions in `apps/filament-dryer-control/ios/` and `apps/filament-dryer-control/android/`

**Checkpoint**: US1 green with FakePhilarmonyWsClient

---

## Phase 4: User Story 2 — Real-Time Status Dashboard (Priority: P1)

**Goal**: `status/update` live fields, badges, 60‑min charts, units (FR-002)

**Independent Test**: Mocked 1Hz stream renders dashboard; charts scroll; °C/°F; sensor-error badge

### Tests

- [ ] T034 [P] [US2] Dashboard flow test in `apps/filament-dryer-control/test/features/dashboard/dashboard_flow_test.dart`

### Implementation

- [ ] T035 [P] [US2] Telemetry ring buffer in `apps/filament-dryer-control/lib/device/telemetry_buffer.dart`
- [ ] T036 [US2] Dashboard cards + progress ring in `apps/filament-dryer-control/lib/features/dashboard/dashboard_page.dart`
- [ ] T037 [P] [US2] Live charts (`fl_chart`) in `apps/filament-dryer-control/lib/features/dashboard/live_charts.dart`
- [ ] T038 [P] [US2] Unit format helpers in `apps/filament-dryer-control/lib/features/settings/unit_format.dart`
- [ ] T039 [US2] Fault banner (`status/fault`) in `apps/filament-dryer-control/lib/features/dashboard/fault_banner.dart`
- [ ] T040 [US2] Expanded master–detail layout in `apps/filament-dryer-control/lib/features/dashboard/dashboard_page.dart`

**Checkpoint**: Dashboard works compact + expanded with fake telemetry

---

## Phase 5: User Story 3 — Drying Cycle Control Stop-only (Priority: P1)

**Goal**: Start + **Stop** with confirm; no Pause UI (FR-003 clarify)

**Independent Test**: Start sends `control/start` with `max_duration_min`; bounds blocked client-side; Stop confirms → `control/stop`; UI has **no** Pause/Resume controls

### Tests

- [ ] T041 [P] [US3] Start/stop flow test (assert no pause affordance) in `apps/filament-dryer-control/test/features/cycle/cycle_control_flow_test.dart`

### Implementation

- [ ] T042 [US3] Start cycle form in `apps/filament-dryer-control/lib/features/cycle/start_cycle_page.dart`
- [ ] T043 [US3] Stop confirmation + actions only (no pause) in `apps/filament-dryer-control/lib/features/cycle/cycle_actions.dart`
- [ ] T044 [US3] Wire core validators before send in `apps/filament-dryer-control/lib/features/cycle/cycle_controller.dart`
- [ ] T045 [US3] Cycle result screen on completion in `apps/filament-dryer-control/lib/features/cycle/cycle_result_page.dart`
- [ ] T046 [US3] Persist in-progress `DryingCycle` on start in `apps/filament-dryer-control/lib/data/drying_cycle_repository.dart`

**Checkpoint**: Start/stop journey green; Pause absent from UI

---

## Phase 6: User Story 4 — Device Configuration (Priority: P2)

**Goal**: `config/hardware` (+ related); pin validate; ack/errors (FR-004)

**Independent Test**: Conflicts blocked; ack success path; error topic surfaced

### Tests

- [ ] T047 [P] [US4] Config flow test in `apps/filament-dryer-control/test/features/config/config_flow_test.dart`

### Implementation

- [ ] T048 [P] [US4] HardwareConfig helpers in `packages/philarmony_core/lib/src/models/hardware_config.dart` + export
- [ ] T049 [US4] Config UI (sensors/pins/display) in `apps/filament-dryer-control/lib/features/config/`
- [ ] T050 [US4] PinValidator before send in `apps/filament-dryer-control/lib/features/config/config_controller.dart`
- [ ] T051 [US4] Handle `config/hardware/response` + `error` in `apps/filament-dryer-control/lib/device/philarmony_ws_client.dart`
- [ ] T052 [P] [US4] WiFi section in `apps/filament-dryer-control/lib/features/config/wifi_section.dart`
- [ ] T053 [P] [US4] Advanced read-only in `apps/filament-dryer-control/lib/features/config/advanced_section.dart`

**Checkpoint**: Config journey independent with mock ack/error

---

## Phase 7: User Story 5 — Material Profiles (Priority: P2)

**Goal**: Profile sync CRUD; start-with-profile; offline queue (FR-008)

**Independent Test**: List from mock profiles response; customs CRUD; builtins RO; start with `profile_id`

### Tests

- [ ] T054 [P] [US5] Profiles flow test in `apps/filament-dryer-control/test/features/profiles/profiles_flow_test.dart`

### Implementation

- [ ] T055 [US5] Profile sync service in `apps/filament-dryer-control/lib/features/profiles/profile_sync_service.dart`
- [ ] T056 [US5] Profiles UI in `apps/filament-dryer-control/lib/features/profiles/profiles_page.dart`
- [ ] T057 [US5] Profile picker on start in `apps/filament-dryer-control/lib/features/cycle/start_cycle_page.dart`
- [ ] T058 [US5] Offline edits → `PendingCommand` in `apps/filament-dryer-control/lib/data/pending_command_repository.dart`
- [ ] T059 [US5] Flush pending on reconnect in `apps/filament-dryer-control/lib/device/session_providers.dart`

**Checkpoint**: Profiles online + queue offline

---

## Phase 8: User Story 6 — History & CSV+PDF Export (Priority: P2)

**Goal**: History list/detail/stats + **CSV and PDF** exports (FR-005 + `contracts/history-export.md`)

**Independent Test**: Seeded cycle → list/detail offline; CSV file; PDF opens with summary + charts

### Tests

- [ ] T060 [P] [US6] History list offline flow in `apps/filament-dryer-control/test/features/history/history_flow_test.dart`
- [ ] T061 [P] [US6] Export CSV+PDF unit/flow test in `apps/filament-dryer-control/test/features/history/cycle_export_test.dart`

### Implementation

- [ ] T062 [US6] Finalize cycle + downsample samples in `apps/filament-dryer-control/lib/data/drying_cycle_repository.dart`
- [ ] T063 [US6] History list + filters in `apps/filament-dryer-control/lib/features/history/history_page.dart`
- [ ] T064 [P] [US6] Cycle detail charts in `apps/filament-dryer-control/lib/features/history/cycle_detail_page.dart`
- [ ] T065 [US6] CSV exporter in `apps/filament-dryer-control/lib/features/history/cycle_csv_export.dart`
- [ ] T066 [US6] PDF exporter (summary + charts) in `apps/filament-dryer-control/lib/features/history/cycle_pdf_export.dart` per `contracts/history-export.md`
- [ ] T067 [P] [US6] Stats widgets in `apps/filament-dryer-control/lib/features/history/history_stats.dart`
- [ ] T068 [US6] Expanded master–detail history in `apps/filament-dryer-control/lib/features/history/history_page.dart`
- [ ] T069 [US6] Export actions wired in UI (CSV + PDF) in `apps/filament-dryer-control/lib/features/history/cycle_detail_page.dart`

**Checkpoint**: Both export formats succeed in automated test with fixtures

---

## Phase 9: User Story 7 — Multi-Device Management (Priority: P3)

**Goal**: Registry, switcher, max 3 sessions (FR-006)

**Independent Test**: Two `KnownDevice`s; switcher changes dashboard; max concurrent enforced

### Tests

- [ ] T070 [P] [US7] Multi-session flow in `apps/filament-dryer-control/test/features/devices/multi_device_flow_test.dart`

### Implementation

- [ ] T071 [US7] Session manager (default max 3) in `apps/filament-dryer-control/lib/device/session_manager.dart`
- [ ] T072 [US7] Device switcher in `apps/filament-dryer-control/lib/shell/device_switcher.dart`
- [ ] T073 [US7] Per-device notification toggles in `apps/filament-dryer-control/lib/features/devices/device_settings_page.dart`
- [ ] T074 [US7] Online/offline indicators in `apps/filament-dryer-control/lib/features/discover/discover_page.dart`

**Checkpoint**: Multi-device does not break US1–US6

---

## Phase 10: Background WS, Notifications & Polish

**Purpose**: Required mobile background WS (clarify), local notifications, packaging, CI, a11y, handoff to 005

- [ ] T075 Implement Android foreground service (`dataSync`) background WS in `apps/filament-dryer-control/lib/platform/background_session_android.dart` + `android/` manifests
- [ ] T076 Implement iOS background session/entitlements for sustained WS in `apps/filament-dryer-control/lib/platform/background_session_ios.dart` + `ios/Runner/*`
- [ ] T077 Wire `BackgroundSession` start/stop with active cycle in `apps/filament-dryer-control/lib/device/session_providers.dart` (surface OS denial errors)
- [ ] T078 [P] Widget/integration test for background policy hooks (mocked) in `apps/filament-dryer-control/test/platform/background_session_test.dart`
- [ ] T079 [P] Local notifications adapter in `apps/filament-dryer-control/lib/platform/local_notifications.dart`
- [ ] T080 [P] Settings page (locale, units, PIN stub, notification prefs) in `apps/filament-dryer-control/lib/features/settings/settings_page.dart`
- [ ] T081 [P] Accessibility labels pass on primary screens under `apps/filament-dryer-control/lib/features/`
- [ ] T082 Complete ARB PT-BR/EN-US in `apps/filament-dryer-control/lib/l10n/`
- [ ] T083 [P] Desktop packaging tree `apps/filament-dryer-control/packaging/` (DEC-011 patterns)
- [ ] T084 Wire `make package-control*` in root `Makefile`
- [ ] T085 [P] CI `flutter analyze` + `flutter test` in `.github/workflows/control-app-ci.yml`
- [ ] T086 Document iOS/Android background entitlements + store notes in `apps/filament-dryer-control/README.md`
- [ ] T087 Register open manual stages VS-1…VS-9 from `quickstart.md` into `specs/005-manual-validation/` (no open manuals left on 003)
- [ ] T088 [P] Ensure root `README.md` lists control-app paths
- [ ] T089 Dashboard rebuild performance pass in `apps/filament-dryer-control/lib/features/dashboard/`

**Checkpoint**: CI green; bg WS implementations present both platforms; manuals only under 005

---

## Dependencies & Execution Order

### Phase Dependencies

- Setup → Foundational (blocks stories) → US1 → US2 → US3 (MVP)  
- US4 parallelizable with US3 after US1  
- US5 after US3 start form  
- US6 after US3 persistence (+ PDF/CSV)  
- US7 after US1  
- Phase 10 after desired stories (bg WS can start after US3)

### User Story Dependencies

| Story | Depends on | Independently testable with |
|-------|------------|-----------------------------|
| US1 Discovery | Foundational | Fake WS + fake mDNS |
| US2 Dashboard | US1 session | Fake telemetry |
| US3 Cycle Stop-only | US1 | Mock control topics |
| US4 Config | US1 | Mock config ack/error |
| US5 Profiles | US1 (+ US3) | Mock profile topics |
| US6 History+Export | US3 | Seeded Drift + export fixtures |
| US7 Multi-device | US1 | Two fake sessions |

### Parallel Opportunities

- Phase 1: T002–T006, T008 after T001  
- Phase 2: T009–T012, T019–T020, T022–T023  
- US6: T064/T065/T067 parallel; T061 with T060  
- Phase 10: T075∥T076; T079–T081; T083∥T085

---

## Parallel Example: User Story 1

```bash
Task: "connect_flow_test.dart"
Task: "ws_client_test.dart"
Task: "mdns_discovery.dart"
Task: "known_device_repository.dart"
```

## Parallel Example: User Story 6

```bash
Task: "cycle_csv_export.dart"
Task: "cycle_pdf_export.dart"
Task: "history_stats.dart"
Task: "cycle_export_test.dart"
```

---

## Implementation Strategy

### MVP First

1. Phase 1–2  
2. US1–US3 (discover, dashboard, start/stop)  
3. Complete **T075–T078** (bg WS iOS+Android) and **T060–T069** (CSV+PDF; at least T061, T065, T066, T069) before MVP sign-off  
4. Manual VS-1/2/7/8 → `005` via T087  

Demo can stop after US3; **acceptance “MVP”** = Core UX + bg WS + exports.

### Incremental

US4 → US5 → US6 (if not already in MVP gate) → US7 → remaining Phase 10 packaging/CI

### Suggested MVP Scope (product-complete gate)

**Core UX:** T001–T046 (Setup + Foundational + US1–US3)

**Also required before calling MVP “done” (spec clarify locks):**
- Background WS iOS+Android: **T075–T078** (+ T086 docs)
- History CSV+PDF: **T060–T069** (at least T061, T065, T066, T069)

Ship demo can stop after US3; **acceptance “MVP”** = Core UX + bg WS + exports.

---

## Notes

- Canonical WS: port **80** `/ws`; topics per `001`  
- Entity **`KnownDevice`** — never overload installer `DeviceProfile`  
- **No Pause/Resume** controls in MVP  
- PDF is **required** with CSV (T065–T066, T061)  
- Background WS is **required** on iOS and Android (T075–T078), not best-effort
- MVP acceptance gate includes Core UX + bg WS + CSV/PDF (see Suggested MVP Scope)  
- Commit only with explicit user authorization

**Constitution compliance:** Philarmony; automated tests on 003; open manuals in 005; bilingual; GPLv3
