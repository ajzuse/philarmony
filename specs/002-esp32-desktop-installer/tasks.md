# Tasks: ESP32 Desktop Installer

**Input**: Design documents from `/specs/002-esp32-desktop-installer/`  
**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`  
**Stack**: Flutter desktop + `packages/philarmony_core` + bundled esptool (DEC-010); host packages MSIX/DMG/AppImage/deb/rpm via Make (DEC-011)

**Tests**: Not requested as TDD in the spec; add focused `flutter_test`/`dart test` where noted in Polish (plan + constitution automated-testing gate).

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `- [ ] [ID] [P?] [Story?] Description with file path`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete work)
- **[Story]**: [US1]…[US5] maps to scenarios / FR groups below

## Path Conventions

- Shared domain: `packages/philarmony_core/`
- Flutter app: `apps/esp32-desktop-installer/`
- Packaging: `apps/esp32-desktop-installer/packaging/`, root `Makefile`, `.github/workflows/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Monorepo Flutter scaffold, assets layout, workspace tooling

- [ ] T001 Create Dart package skeleton in `packages/philarmony_core/pubspec.yaml` and `packages/philarmony_core/lib/philarmony_core.dart`
- [ ] T002 Create Flutter desktop app scaffold (windows/macos/linux enabled) in `apps/esp32-desktop-installer/pubspec.yaml` depending on `philarmony_core`
- [ ] T003 [P] Add asset directories and placeholders in `apps/esp32-desktop-installer/assets/firmware/` and `apps/esp32-desktop-installer/assets/tools/{windows,macos,linux}/`
- [ ] T004 [P] Ignore generated installers and Flutter build outputs in `.gitignore` (`apps/esp32-desktop-installer/dist/`, `build/`)
- [ ] T005 [P] Document Flutter SDK + desktop + packaging host deps in `.vscode/workspace.json`
- [ ] T006 [P] Add empty packaging tree in `apps/esp32-desktop-installer/packaging/{windows,macos,linux}/` and `apps/esp32-desktop-installer/packaging/README.md`

**Checkpoint**: `flutter pub get` succeeds for app + core; app can `flutter run` empty shell on at least one desktop OS

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Shared models, validation, NVS mapping, flash/serial abstractions — MUST complete before user stories

**⚠️ CRITICAL**: No user story UI/flash work until this phase completes

- [ ] T007 [P] Implement DeviceProfile / SensorConfig / PinMapping / DisplayConfig / WiFiConfig models in `packages/philarmony_core/lib/src/models/`
- [ ] T008 [P] Implement FilamentProfile + built-in PLA/PETG/ABS/TPU/Nylon defaults in `packages/philarmony_core/lib/src/models/filament_profile.dart`
- [ ] T009 [P] Implement FirmwarePackage + FlashJob + InstallerSession models in `packages/philarmony_core/lib/src/models/`
- [ ] T010 Implement PinValidator (duplicates, reserved/strapping, PWM capability by chip) in `packages/philarmony_core/lib/src/validation/pin_validator.dart`
- [ ] T011 [P] Implement DeviceProfile JSON (de)serialization aligned with `specs/002-esp32-desktop-installer/contracts/installer-profile.schema.json` in `packages/philarmony_core/lib/src/serialization/profile_codec.dart`
- [ ] T012 Implement NVS/hardware JSON mapper per `contracts/nvs-config-mapping.md` in `packages/philarmony_core/lib/src/mapping/nvs_config_mapper.dart`
- [ ] T013 [P] Define abstract DeviceDetector + FirmwareFlasher interfaces in `packages/philarmony_core/lib/src/device/`
- [ ] T014 Wire app localization bootstrap (ARB stubs pt-BR/en-US) in `apps/esp32-desktop-installer/lib/l10n/` and `apps/esp32-desktop-installer/l10n.yaml`
- [ ] T015 Create app shell with routing skeleton (wizard steps placeholders) in `apps/esp32-desktop-installer/lib/main.dart` and `apps/esp32-desktop-installer/lib/app.dart`

**Checkpoint**: Foundation ready — `dart test` in core can target validators/mappers; user stories can proceed

---

## Phase 3: User Story 1 — First-Time Device Setup (Priority: P1) 🎯 MVP

**Goal**: USB detect → guided wizard (device, sensors, pins, display, WiFi, review) → flash bundled firmware + NVS → optional WS verify (Scenario 1 / FR-001–005, FR-007–008)

**Independent Test**: Connect ESP32 → complete wizard with valid pins/WiFi → Install → device boots and responds (or flash log shows success stages per `contracts/flash-pipeline.md`)

### Implementation for User Story 1

- [ ] T016 [P] [US1] Implement serial port enumeration via `flutter_libserialport` in `apps/esp32-desktop-installer/lib/device/serial_device_detector.dart`
- [ ] T017 [P] [US1] Implement Device step UI (model, flash size, port select, troubleshooting empty list) in `apps/esp32-desktop-installer/lib/wizard/steps/device_step.dart`
- [ ] T018 [P] [US1] Implement Sensors step UI (temp/humidity types + GPIO) in `apps/esp32-desktop-installer/lib/wizard/steps/sensors_step.dart`
- [ ] T019 [P] [US1] Implement Pins step UI with conflict feedback in `apps/esp32-desktop-installer/lib/wizard/steps/pins_step.dart`
- [ ] T020 [P] [US1] Implement Display step UI (driver, resolution, status fields) in `apps/esp32-desktop-installer/lib/wizard/steps/display_step.dart`
- [ ] T021 [P] [US1] Implement WiFi step UI (SSID, password show/hide, optional static IP) in `apps/esp32-desktop-installer/lib/wizard/steps/wifi_step.dart`
- [ ] T022 [US1] Implement Review step summarizing DeviceProfile + validation gate in `apps/esp32-desktop-installer/lib/wizard/steps/review_step.dart`
- [ ] T023 [US1] Implement InstallerSessionController (step machine, Back/Next, validation_errors) in `apps/esp32-desktop-installer/lib/wizard/installer_session_controller.dart`
- [ ] T024 [US1] Bundle esptool binaries + document provenance in `apps/esp32-desktop-installer/assets/tools/` and `apps/esp32-desktop-installer/packaging/README.md`
- [ ] T025 [US1] Add script/Make target to copy firmware `001` build artifacts into `apps/esp32-desktop-installer/assets/firmware/` from PlatformIO output
- [ ] T026 [US1] Implement FirmwareFlasher (esptool Process, stage parse, progress stream) in `apps/esp32-desktop-installer/lib/flash/esptool_firmware_flasher.dart`
- [ ] T027 [US1] Implement NVS/config image writer using mapper output in `apps/esp32-desktop-installer/lib/flash/nvs_image_builder.dart`
- [ ] T028 [US1] Implement Flash step UI (progress, stages, Retry without restore claim) in `apps/esp32-desktop-installer/lib/wizard/steps/flash_step.dart`
- [ ] T029 [US1] Implement soft post-flash WS/HTTP verifier (`ok`/`skipped`/`warn`; never fails Flash Success) in `apps/esp32-desktop-installer/lib/flash/post_flash_verifier.dart`
- [ ] T030 [US1] Wire wizard navigation end-to-end in `apps/esp32-desktop-installer/lib/wizard/wizard_page.dart`

**Checkpoint**: First-time setup MVP works on one desktop OS with real or mocked flasher

---

## Phase 4: User Story 2 — Filament Drying Profiles (Priority: P2)

**Goal**: View built-ins, create/edit/delete customs (≤20), sync into flash NVS (FR-006; wizard `profiles` step)

**Independent Test**: Add custom profile → appear in Review → after flash, firmware exposes profile (or NVS image contains profile payload per mapper)

### Implementation for User Story 2

- [ ] T031 [P] [US2] Implement Profiles step UI (built-in read-only, custom CRUD + validation) in `apps/esp32-desktop-installer/lib/wizard/steps/profiles_step.dart`
- [ ] T032 [US2] Enforce max 20 customs + temp/duration/humidity ranges in `packages/philarmony_core/lib/src/validation/filament_profile_validator.dart`
- [ ] T033 [US2] Include filament_profiles in NVS mapper path in `packages/philarmony_core/lib/src/mapping/nvs_config_mapper.dart`
- [ ] T034 [US2] Insert Profiles step into wizard order in `apps/esp32-desktop-installer/lib/wizard/installer_session_controller.dart`

**Checkpoint**: Profiles editable and included in flash payload

---

## Phase 5: User Story 3 — Configuration Profile Export/Import (Priority: P2)

**Goal**: Save/load JSON profiles for bulk flashing; WiFi password stripped/placeholder (Scenario 3 / FR-009)

**Independent Test**: Export after Review → wipe session → Import → password re-entry required → flash succeeds

### Implementation for User Story 3

- [ ] T035 [P] [US3] Implement ProfileStore export/import with password redaction in `packages/philarmony_core/lib/src/serialization/profile_store.dart`
- [ ] T036 [US3] Add Export/Import actions + file picker in `apps/esp32-desktop-installer/lib/wizard/profile_io_actions.dart`
- [ ] T037 [US3] Validate imported JSON against schema version and surface errors in `apps/esp32-desktop-installer/lib/wizard/installer_session_controller.dart`
- [ ] T038 [US3] Block flash until WiFi password re-entered after import in `apps/esp32-desktop-installer/lib/wizard/steps/wifi_step.dart`

**Checkpoint**: Bulk config via JSON works without leaking passwords to disk

---

## Phase 6: User Story 4 — Reconfiguration of Existing Device (Priority: P3)

**Goal**: Reconnect previously flashed device, load **local** last session / JSON, adjust settings, re-flash (Scenario 2). No on-device NVS read.

**Independent Test**: After one successful flash, relaunch → Load last session → change pin/WiFi → Retry flash applies new config (USB never dumps NVS)

### Implementation for User Story 4

- [ ] T039 [P] [US4] Implement chip/port identity helpers for UX only (no NVS dump) in `apps/esp32-desktop-installer/lib/device/chip_info_reader.dart`
- [ ] T040 [US4] Add “Load last session / New setup” entry in `apps/esp32-desktop-installer/lib/wizard/home_entry_page.dart`
- [ ] T041 [US4] Persist last successful DeviceProfile (sans password) to local app support dir in `apps/esp32-desktop-installer/lib/persistence/last_profile_store.dart`
- [ ] T042 [US4] Confirm overwrite/re-erase messaging before flash in `apps/esp32-desktop-installer/lib/wizard/steps/review_step.dart`

**Checkpoint**: Reconfigure path usable without forcing full blank wizard every time

---

## Phase 7: User Story 5 — Host Application Distribution (Priority: P1 for release)

**Goal**: Single-download installers — Windows MSIX, macOS DMG, Linux AppImage + deb + rpm via Make (FR-012 / DEC-011 / `contracts/desktop-distribution.md`)

**Independent Test**: `make package-installer-linux` produces three artifacts; MSIX/DMG build docs runnable; install on clean machine launches app (VS-4/VS-5 in quickstart)

### Implementation for User Story 5

- [ ] T043 [P] [US5] Configure Windows MSIX (`msix` / `msix_config`) in `apps/esp32-desktop-installer/pubspec.yaml` and `apps/esp32-desktop-installer/packaging/windows/`
- [ ] T044 [P] [US5] Configure macOS DMG packaging + notarization checklist (REQUIRED for public) in `apps/esp32-desktop-installer/packaging/macos/dmg.json` and packaging README
- [ ] T045 [P] [US5] Configure Linux AppImage metadata in `apps/esp32-desktop-installer/packaging/linux/appimage.yml`
- [ ] T046 [P] [US5] Configure Linux `.deb` metadata in `apps/esp32-desktop-installer/packaging/linux/deb/`
- [ ] T047 [P] [US5] Configure Linux `.rpm` metadata (Fedora/RHEL-family) in `apps/esp32-desktop-installer/packaging/linux/rpm/`
- [ ] T048 [US5] Add Makefile targets `package-installer-linux`, `package-installer-linux-appimage`, `package-installer-linux-deb`, `package-installer-linux-rpm` in `Makefile`
- [ ] T049 [US5] Add optional `package-installer-windows` / `package-installer-macos` Make wrappers documenting host OS requirements in `Makefile`
- [ ] T050 [US5] Implement CI workflow attaching MSIX/DMG/AppImage/deb/rpm in `.github/workflows/desktop-installer-release.yml`
- [ ] T051 [US5] Document Download & Install (Win/macOS/`apt`/`dnf`/AppImage) in `docs/PT-BR/instalador.md` and `docs/EN-US/installer.md`
- [ ] T052 [US5] Add HostReleaseArtifact naming/checksum notes to release checklist in `apps/esp32-desktop-installer/packaging/README.md`

**Checkpoint**: Unsigned CI packages build; public signing secrets documented as follow-up when available

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: i18n completeness, diagnostics, docs sync, automated tests, quickstart validation

- [ ] T053 [P] Complete PT-BR and EN-US ARB strings for all wizard/errors in `apps/esp32-desktop-installer/lib/l10n/`
- [ ] T054 [P] Implement flash log panel + export in `apps/esp32-desktop-installer/lib/wizard/widgets/flash_log_panel.dart` (FR-011)
- [ ] T055 [P] Add unit tests for PinValidator and ProfileStore redaction in `packages/philarmony_core/test/`
- [ ] T056 [P] Add widget/flow tests with mocked FirmwareFlasher in `apps/esp32-desktop-installer/test/`
- [ ] T057 [P] Add third-party notices for bundled esptool in `apps/esp32-desktop-installer/NOTICES.md`
- [ ] T058 Sync README Desktop Installer section + stack table with DEC-010/011 in `README.md`
- [ ] T059 Run `specs/002-esp32-desktop-installer/quickstart.md` validation checklist (VS-1…VS-6) and record gaps in `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md`
- [ ] T060 [P] Optional Inno Setup `.exe` fallback config in `apps/esp32-desktop-installer/packaging/windows/inno/` if MSIX sideload blocked

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundational (Phase 2)**: Depends on Setup — **BLOCKS** all user stories
- **US1 (Phase 3)**: After Foundational — **MVP**
- **US2 (Phase 4)**: After US1 wizard shell (needs session controller + flash mapper)
- **US3 (Phase 5)**: After Foundational models/codec; best after US1 Review exists
- **US4 (Phase 6)**: After US1 flash path
- **US5 (Phase 7)**: Packaging configs after Setup; Make/CI need buildable app; full smoke needs US1 runnable
- **Polish (Phase 8)**: After desired stories complete

### User Story Dependencies

```text
Phase1 → Phase2 → US1 (MVP)
                 ↘ US3 (profiles JSON)  [can start after T011]
                 ↘ US5 (host pkgs)      [packaging files // after T002]
         US1 → US2 (filament step)
         US1 → US4 (reconfigure)
         US1+US5 → Polish
```

### Parallel Opportunities

- T003–T006 in Setup
- T007–T009, T011, T013 in Foundational
- T017–T021 wizard steps in US1
- T043–T047 packaging configs in US5
- T053–T057 polish items

---

## Parallel Example: User Story 1

```bash
# After T023 session controller exists, wizard steps in parallel:
Task: "Implement Device step UI in apps/esp32-desktop-installer/lib/wizard/steps/device_step.dart"
Task: "Implement Sensors step UI in apps/esp32-desktop-installer/lib/wizard/steps/sensors_step.dart"
Task: "Implement Pins step UI in apps/esp32-desktop-installer/lib/wizard/steps/pins_step.dart"
Task: "Implement Display step UI in apps/esp32-desktop-installer/lib/wizard/steps/display_step.dart"
Task: "Implement WiFi step UI in apps/esp32-desktop-installer/lib/wizard/steps/wifi_step.dart"
```

## Parallel Example: User Story 5

```bash
Task: "Configure Windows MSIX in apps/esp32-desktop-installer/packaging/windows/"
Task: "Configure macOS DMG in apps/esp32-desktop-installer/packaging/macos/dmg.json"
Task: "Configure Linux AppImage in apps/esp32-desktop-installer/packaging/linux/appimage.yml"
Task: "Configure Linux deb in apps/esp32-desktop-installer/packaging/linux/deb/"
Task: "Configure Linux rpm in apps/esp32-desktop-installer/packaging/linux/rpm/"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Phase 1 Setup → Phase 2 Foundational  
2. Phase 3 US1 (detect → wizard → flash → verify)  
3. **STOP and VALIDATE** on hardware per quickstart VS-1  
4. Then US5 packaging so non-devs can install the app  

### Incremental Delivery

1. Setup + Foundational  
2. US1 → demo first flash  
3. US2 filament profiles  
4. US3 JSON bulk  
5. US4 reconfigure  
6. US5 host installers (Make Linux + MSIX/DMG)  
7. Polish i18n/logs/docs/tests  

### Suggested MVP Scope

**US1 only** (T001–T030), then prioritize **US5** (T043–T052) so the app is distributable; US2–US4 next.

---

## Notes

- [P] = different files, no incomplete-task dependencies  
- Do not flash without PinValidator success (failsafe)  
- WiFi passwords never written to exported JSON  
- Linux end users install `.rpm`/`.deb`/AppImage; `make package-installer-linux*` is for builders/CI  
- Constitution: Philarmony; bilingual docs; commit only with explicit user approval  

**Constitution compliance:** PROJECT_NAME = Philarmony, provisions per `.specify/memory/constitution.md`
