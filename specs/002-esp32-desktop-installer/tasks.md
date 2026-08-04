# Tasks: ESP32 Desktop Installer

**Input**: Design documents from `/specs/002-esp32-desktop-installer/`  
**Prerequisites**: `plan.md`, `spec.md` (Clarifications 2026-07-28), `research.md` (R8–R12), `data-model.md`, `contracts/`  
**Stack**: Flutter **3.44.0 via FVM** (`.fvmrc`) + `philarmony_core` + bundled esptool (DEC-010/011)

**Progress note (2026-07-29)**: Implementation essentially complete except hardware VS-1 smoke (T031) and final quickstart VS-1/4/5 sign-off (T062). Run `make bundle-esptool` + `make sync-installer-firmware` before real flash.

**Tests**: Polish includes `make test-flutter` (already green for core + wizard/widget). Historical `[x]` automated tasks stay here as history. **New automated work → `specs/005-automated-flow-testing`**. Open remnants below are **manual only** (T031, T062).

**Test-policy audit (2026-08-04)**: No open automated-test tasks remain on this feature. T031/T062 classified `[MANUAL]` per Constitution v0.8.0.

## Format: `- [ ] [ID] [P?] [Story?] Description with file path`

- **[P]**: Parallelizable  
- **[Story]**: [US1]…[US5]

## Path Conventions

- Shared: `packages/philarmony_core/`
- App: `apps/esp32-desktop-installer/`
- Tooling: root `Makefile`, `.fvmrc`, `.github/workflows/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Monorepo + FVM + desktop runners

- [x] T001 Create Dart package skeleton in `packages/philarmony_core/pubspec.yaml` and `packages/philarmony_core/lib/philarmony_core.dart`
- [x] T002 Create Flutter desktop app in `apps/esp32-desktop-installer/pubspec.yaml` depending on `philarmony_core`
- [x] T003 [P] Add asset placeholders in `apps/esp32-desktop-installer/assets/firmware/` and `apps/esp32-desktop-installer/assets/tools/{windows,macos,linux}/`
- [x] T004 [P] Ignore Flutter/FVM/dist outputs in `.gitignore`
- [x] T005 [P] Document Flutter/FVM + packaging in `.vscode/workspace.json` and `.vscode/settings.json` (`dart.flutterSdkPath` → `.fvm/flutter_sdk`)
- [x] T006 [P] Add packaging tree in `apps/esp32-desktop-installer/packaging/`
- [x] T007 Pin Flutter **3.44.0** via FVM in `.fvmrc` and `.fvm/fvm_config.json`
- [x] T008 Generate desktop runners (`macos/`, `windows/`, `linux/`) under `apps/esp32-desktop-installer/`
- [x] T009 Add Make shortcuts `run`, `run-macos`, `devices`, `doctor`, `pub-get`, `test-flutter`, `create-platforms` in `Makefile`

**Checkpoint**: `make run` / `make test-flutter` work with FVM

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Shared models and validation

- [x] T010 [P] DeviceProfile / SensorConfig / PinMapping / DisplayConfig / WiFiConfig in `packages/philarmony_core/lib/src/models/`
- [x] T011 [P] FilamentProfile builtins in `packages/philarmony_core/lib/src/models/filament_profile.dart`
- [x] T012 [P] FirmwarePackage + FlashJob (`network_verify`) + InstallerSession in `packages/philarmony_core/lib/src/models/`
- [x] T013 PinValidator in `packages/philarmony_core/lib/src/validation/pin_validator.dart`
- [x] T014 [P] ProfileCodec aligned with `contracts/installer-profile.schema.json` in `packages/philarmony_core/lib/src/serialization/profile_codec.dart`
- [x] T015 NvsConfigMapper in `packages/philarmony_core/lib/src/mapping/nvs_config_mapper.dart`
- [x] T016 [P] DeviceDetector / FirmwareFlasher / PostFlashVerifier interfaces in `packages/philarmony_core/lib/src/device/`
- [x] T017 ARB stubs + app shell in `apps/esp32-desktop-installer/lib/l10n/` and `apps/esp32-desktop-installer/lib/app.dart`
- [x] T018 Unit tests PinValidator + ProfileStore in `packages/philarmony_core/test/core_test.dart`

**Checkpoint**: `make test-core` passes

---

## Phase 3: User Story 1 — First-Time Device Setup (Priority: P1) 🎯 MVP

**Goal**: Detect → wizard → flash → Flash Success = esptool verify; soft network check (R8/R11)

**Independent Test**: Wizard blocks bad pins; with port + bundled esptool + firmware.bin, flash stages complete; unreachable WS → warning only

### Done

- [x] T019 [P] [US1] Serial enumeration via `flutter_libserialport` in `apps/esp32-desktop-installer/lib/device/serial_device_detector.dart`
- [x] T020 [P] [US1] Device/Sensors/Pins/Display/WiFi/Review/Flash steps under `apps/esp32-desktop-installer/lib/wizard/steps/`
- [x] T021 [US1] InstallerSessionController in `apps/esp32-desktop-installer/lib/wizard/installer_session_controller.dart`
- [x] T022 [US1] WizardPage navigation in `apps/esp32-desktop-installer/lib/wizard/wizard_page.dart`
- [x] T023 [US1] EsptoolFirmwareFlasher + SoftPostFlashVerifier in `apps/esp32-desktop-installer/lib/flash/`
- [x] T024 [US1] NvsImageBuilder (JSON sidecar MVP) in `apps/esp32-desktop-installer/lib/flash/nvs_image_builder.dart`
- [x] T025 [US1] macOS USB entitlements (sandbox off) in `apps/esp32-desktop-installer/macos/Runner/*entitlements`
- [x] T026 [US1] Flow/widget tests in `apps/esp32-desktop-installer/test/`

### Remaining

- [x] T027 [US1] Bundle platform esptool binaries into `apps/esp32-desktop-installer/assets/tools/{macos,linux,windows}/` and document checksums in `apps/esp32-desktop-installer/packaging/README.md`
- [x] T028 [US1] Wire `make sync-installer-firmware` output path into flasher default package in `apps/esp32-desktop-installer/lib/flash/esptool_firmware_flasher.dart` (resolve asset/`path_provider` absolute paths)
- [x] T029 [US1] Replace JSON sidecar with firmware-compatible NVS/config partition image per `contracts/nvs-config-mapping.md` in `apps/esp32-desktop-installer/lib/flash/nvs_image_builder.dart`
- [x] T030 [US1] Persist selected serial port across steps in `apps/esp32-desktop-installer/lib/wizard/installer_session_controller.dart` (avoid FlashStep default `/dev/ttyUSB0`)
- [ ] T031 [US1] [MANUAL] Hardware smoke VS-1 on real ESP32; record in `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md`

**Checkpoint**: End-to-end USB flash on one desktop OS

---

## Phase 4: User Story 2 — Filament Profiles (Priority: P2)

**Goal**: Built-ins + customs ≤20 in flash payload (FR-006)

**Independent Test**: Custom profile appears in Review and mapper blob

- [x] T032 [P] [US2] Profiles step UI in `apps/esp32-desktop-installer/lib/wizard/steps/profiles_step.dart`
- [x] T033 [US2] FilamentProfileValidator in `packages/philarmony_core/lib/src/validation/filament_profile_validator.dart`
- [x] T034 [US2] filament_profiles in NvsConfigMapper
- [x] T035 [US2] Edit-custom-profile dialog (name/temp/duration/humidity) in `apps/esp32-desktop-installer/lib/wizard/steps/profiles_step.dart` (create-only today)
- [x] T036 [US2] Unit tests for FilamentProfileValidator in `packages/philarmony_core/test/filament_profile_validator_test.dart`

**Checkpoint**: Full CRUD for customs with validation errors surfaced in UI

---

## Phase 5: User Story 3 — Profile Export/Import (Priority: P2)

**Goal**: JSON bulk flash; password always `***`/omit (R10)

**Independent Test**: Export has no real password; import requires re-entry before flash

- [x] T037 [P] [US3] ProfileStore redaction in `packages/philarmony_core/lib/src/serialization/profile_store.dart`
- [x] T038 [US3] Export/Import actions in `apps/esp32-desktop-installer/lib/wizard/profile_io_actions.dart`
- [x] T039 [US3] Save export to user-chosen `.json` file (not only clipboard) via `file_picker` in `apps/esp32-desktop-installer/lib/wizard/profile_io_actions.dart`
- [x] T040 [US3] Surfaced import schema errors in UI banner in `apps/esp32-desktop-installer/lib/wizard/wizard_page.dart`

**Checkpoint**: File-based export/import round-trip

---

## Phase 6: User Story 4 — Local Reconfigure (Priority: P3)

**Goal**: Load last session / import — no USB NVS dump (R9)

**Independent Test**: After flash → relaunch → Load last session → password empty → reflash

- [x] T041 [P] [US4] ChipInfoReader UX-only in `apps/esp32-desktop-installer/lib/device/chip_info_reader.dart`
- [x] T042 [US4] HomeEntryPage New/Load in `apps/esp32-desktop-installer/lib/wizard/home_entry_page.dart`
- [x] T043 [US4] LastProfileStore in `apps/esp32-desktop-installer/lib/persistence/last_profile_store.dart`
- [x] T044 [US4] Home “Import profile…” entry calling file picker in `apps/esp32-desktop-installer/lib/wizard/home_entry_page.dart`
- [x] T045 [US4] Unit/integration test for LastProfileStore redaction in `apps/esp32-desktop-installer/test/last_profile_store_test.dart`

**Checkpoint**: Three entry paths: New / Last / Import

---

## Phase 7: User Story 5 — Host Distribution (Priority: P1 for release)

**Goal**: Single-download MSIX / notarized DMG / AppImage+deb+rpm via Make (FR-012 / R12)

**Independent Test**: `make package-installer-linux-*` produce files under `dist/`; macOS public path documents notarize; Win/Linux unsigned warnings in docs

### Done

- [x] T046 [P] [US5] Packaging metadata stubs in `apps/esp32-desktop-installer/packaging/{windows,macos,linux}/`
- [x] T047 [US5] Makefile package targets + FVM flutter wiring in `Makefile`
- [x] T048 [US5] CI workflow FVM-aware in `.github/workflows/desktop-installer-release.yml`
- [x] T049 [US5] Download & Install docs in `docs/PT-BR/instalador.md` and `docs/EN-US/installer.md`

### Remaining

- [x] T050 [US5] Implement AppImage wrap script using `apps/esp32-desktop-installer/packaging/linux/appimage.yml` invoked by `package-installer-linux-appimage` in `Makefile`
- [x] T051 [US5] Implement `.deb` build (nfpm/fpm) from `apps/esp32-desktop-installer/packaging/linux/deb/control` in `Makefile`
- [x] T052 [US5] Implement `.rpm` build from `apps/esp32-desktop-installer/packaging/linux/rpm/philarmony-installer.spec` in `Makefile`
- [x] T053 [US5] Smoke `dart run msix:create` after `flutter build windows` on Windows CI / host; store artifact naming in `apps/esp32-desktop-installer/packaging/README.md`
- [x] T054 [US5] macOS DMG script + notarization checklist (secrets) in `apps/esp32-desktop-installer/packaging/macos/` and CI notes
- [x] T055 [P] [US5] Optional Inno Setup config in `apps/esp32-desktop-installer/packaging/windows/inno/`

**Checkpoint**: At least one real host installer artifact per OS family in CI artifacts

---

## Phase 8: Polish & Cross-Cutting

**Purpose**: i18n completeness, UX polish, docs, release readiness

- [x] T056 [P] Flash log panel in `apps/esp32-desktop-installer/lib/wizard/widgets/flash_log_panel.dart`
- [x] T057 [P] NOTICES.md for esptool in `apps/esp32-desktop-installer/NOTICES.md`
- [x] T058 [P] Generate/consume flutter gen-l10n (`AppLocalizations`) instead of hardcoded strings in `apps/esp32-desktop-installer/lib/wizard/`
- [x] T059 [P] Fix DropdownButtonFormField `value`→`initialValue` deprecations in wizard steps under `apps/esp32-desktop-installer/lib/wizard/steps/`
- [x] T060 [P] Visual pinout diagram for selected model in `apps/esp32-desktop-installer/lib/wizard/steps/pins_step.dart` (FR-004)
- [x] T061 Sync README roadmap % and feature status for 002 in `README.md`
- [ ] T062 [MANUAL] Complete quickstart checklist VS-1/VS-4/VS-5 after packaging+HW in `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md`

---

## Dependencies & Execution Order

```text
Done: Phase1–2, most US1–US4 UI, US5 stubs
Next (MVP close): T027–T031 (esptool+firmware+NVS+port+HW)
Then: T050–T054 packaging artifacts
Polish: T058–T062
US2/US3/US4 polish (T035–T045) can parallel packaging
```

### Parallel Opportunities

- T027 // T028 path resolve after assets land  
- T050–T052 Linux formats  
- T035–T036, T039–T040, T044–T045  
- T058–T060 polish UI  

---

## Parallel Example: Remaining US1 flash readiness

```bash
Task: "Bundle esptool into apps/esp32-desktop-installer/assets/tools/..."
Task: "Resolve firmware asset paths in esptool_firmware_flasher.dart"
Task: "Persist selected serial port in installer_session_controller.dart"
```

## Parallel Example: US5 packaging

```bash
Task: "AppImage wrap in Makefile + packaging/linux/appimage.yml"
Task: "deb via nfpm from packaging/linux/deb/control"
Task: "rpm via packaging/linux/rpm/philarmony-installer.spec"
```

---

## Implementation Strategy

### MVP close-out (recommended next)

1. T027–T030 — make USB flash real without hardcoded ports/missing tools  
2. T031 — hardware smoke  
3. T050–T054 — ship host installers  

### Suggested MVP Scope

**US1 remaining (T027–T031)** then **US5 remaining (T050–T054)**.

---

## Notes

- Always use `fvm flutter` / `make run` (Flutter 3.44.0)  
- Flash Success ≠ network reachability  
- Never export real WiFi passwords  
- No on-device NVS dump this phase  
- Public macOS MUST be notarized; Win/Linux unsigned OK MVP with docs  
- Commit only with explicit user approval  

**Constitution compliance:** Philarmony per `.specify/memory/constitution.md`

## Phase 9: Convergence

**Purpose**: Close gaps between spec/plan intent and current implementation (post-implement assessment 2026-07-29). Does not replace open T031/T062 (hardware & checklist sign-off).

- [x] T069 CRITICAL Add GPLv3 license headers to Dart sources under `apps/esp32-desktop-installer/lib/` and `packages/philarmony_core/lib/` per Constitution GPLv3 (missing)
- [x] T063 CRITICAL Flash bootloader.bin + partitions.bin (and sync via `make sync-installer-firmware`) after erase alongside app@0x10000 + NVS@0x9000 in `apps/esp32-desktop-installer/lib/flash/esptool_firmware_flasher.dart` and `Makefile` per FR-008 / US1 (contradicts)
- [x] T064 CRITICAL Fix ESP-IDF-compatible NVS page layout (entry bitmap + CRCs) in `packages/philarmony_core/lib/src/mapping/nvs_binary_writer.dart` or replace with Espressif `nvs_partition_gen`, with Preferences round-trip test per FR-008 / contracts/nvs-config-mapping.md (partial)
- [x] T065 [P] Map dual temp+humidity sensors into Preferences `is_integrated` / `humidity_*` fields in `packages/philarmony_core/lib/src/mapping/nvs_config_mapper.dart` per FR-003 (partial)
- [x] T066 Wire chip model + flash size detection (esptool chip_id/flash_id) into Device step via `apps/esp32-desktop-installer/lib/device/chip_info_reader.dart` and `apps/esp32-desktop-installer/lib/wizard/steps/device_step.dart` per FR-001 (missing)
- [x] T067 Extend `.github/workflows/desktop-installer-release.yml` to run `bundle-esptool` + `sync-installer-firmware` and upload real AppImage/deb/rpm/MSIX/DMG artifacts (not raw Flutter bundles) per FR-012 / plan CI (partial)
- [x] T068 Externalize remaining wizard/step strings to ARB and add in-app PT-BR/EN language selector in `apps/esp32-desktop-installer/lib/app.dart` + `lib/l10n/` + `lib/wizard/` per FR-010 (partial)
- [x] T070 [P] Replace Chip-wrap pinout with model pinout diagram, reserved-pin highlights, and SPI pin fields in `apps/esp32-desktop-installer/lib/wizard/steps/pins_step.dart` per FR-004 (partial)
- [x] T071 [P] Add display resolution presets, status field checkboxes, layout preview, and SPI dc/rst pins in `apps/esp32-desktop-installer/lib/wizard/steps/display_step.dart` per FR-005 (partial)
- [x] T072 [P] Add optional static IP (ip/gateway/netmask/dns) UI + NVS mapping in `apps/esp32-desktop-installer/lib/wizard/steps/wifi_step.dart` and `packages/philarmony_core/` per FR-007 (missing)
- [x] T073 [P] Allow custom profile to shadow builtin id in `apps/esp32-desktop-installer/lib/wizard/steps/profiles_step.dart` per FR-006 (partial)
- [x] T074 Validate imports against `specs/002-esp32-desktop-installer/contracts/installer-profile.schema.json` in `packages/philarmony_core/lib/src/serialization/profile_codec.dart` per FR-009 (partial)
- [x] T075 [P] Add flash log file export and verbose/quiet toggle in `apps/esp32-desktop-installer/lib/wizard/widgets/flash_log_panel.dart` per FR-011 (partial)
- [x] T076 Pass configurable `--baud` (default 921600) to esptool in `apps/esp32-desktop-installer/lib/flash/esptool_firmware_flasher.dart` per FR-001 (missing)
- [x] T077 [P] Add required vs optional field indicators across wizard steps under `apps/esp32-desktop-installer/lib/wizard/steps/` per FR-002 (missing)
- [x] T078 [P] Add advanced custom sensor JSON driver UI (optional) in `apps/esp32-desktop-installer/lib/wizard/steps/sensors_step.dart` per FR-003 (missing)

## Phase 10: Convergence

**Purpose**: Close residual gaps after Phase 9 tasks were marked complete (post-implement reassessment 2026-08-01). Does not replace open T031/T062 (hardware & checklist sign-off).

- [x] T079 CRITICAL Fail CI packaging when AppImage/deb/rpm/MSIX/DMG steps fail (remove `|| true`), stop uploading raw Flutter `bundle`/`Release` trees, and install packaging tools (`appimagetool`/dpkg/rpmbuild) in `.github/workflows/desktop-installer-release.yml` per FR-012 / plan:CI (contradicts)
- [x] T080 CRITICAL Publish GitHub Release attach + SHA-256 checksums for host installer artifacts from `.github/workflows/desktop-installer-release.yml` per FR-012 / contracts/desktop-distribution.md (missing)
- [x] T081 CRITICAL Add complete user-journey flow tests (wizard steps + soft network-verify warn path with mocked flasher) under `apps/esp32-desktop-installer/test/` per Constitution Teste Automatizado / plan:Testing (missing)
- [x] T082 Pass user-supplied/static IP or hotspot host `philarmony`/`192.168.4.1` into `SoftPostFlashVerifier.tryReach` from `apps/esp32-desktop-installer/lib/wizard/steps/flash_step.dart` per FR-008 / US1/AC (partial)
- [x] T083 Externalize remaining hardcoded wizard/step/log strings into ARB (`apps/esp32-desktop-installer/lib/l10n/`) and consume `AppLocalizations` across `lib/wizard/` per FR-010 (partial)
- [x] T084 [P] Ship model-specific pinout maps (ESP32/S2/S3/C3), flash/PSRAM reserved highlights, and PWM/ADC capability checks in `apps/esp32-desktop-installer/lib/wizard/steps/pins_step.dart` + `packages/philarmony_core/lib/src/validation/pin_validator.dart` per FR-004 (partial)
- [x] T085 Load and enforce `specs/002-esp32-desktop-installer/contracts/installer-profile.schema.json` (enums, ranges, required nested fields) in `packages/philarmony_core/lib/src/serialization/profile_codec.dart` per FR-009 (partial)
- [x] T086 Verify firmware + esptool checksums during flash preflight (replace hardcoded `checksumSha256: 'synced'`) in `apps/esp32-desktop-installer/lib/wizard/steps/flash_step.dart` + `lib/flash/esptool_firmware_flasher.dart` per contracts/flash-pipeline.md (missing)
- [x] T087 Wire macOS `notarytool` when Apple secrets are present and keep public-DMG notarization gate hard-fail in `.github/workflows/desktop-installer-release.yml` + `packaging/macos/` per FR-012 (partial)
- [x] T088 [P] Expose configurable flash/probe baud (default 921600) on Device step in `apps/esp32-desktop-installer/lib/wizard/steps/device_step.dart` wired through session → esptool/chip probe per FR-001 (partial)
- [x] T089 [P] Complete required vs optional field indicators and per-step Next validation for device/sensors/display/profiles under `apps/esp32-desktop-installer/lib/wizard/steps/` + `installer_session_controller.dart` per FR-002 (partial)
- [x] T090 Support custom partition table selection/path (or firmware-provided override) in flash pipeline under `apps/esp32-desktop-installer/lib/flash/` per FR-008 (missing)
- [x] T091 [P] Warn on unsupported ESP32 model with supported-variants list in `apps/esp32-desktop-installer/lib/wizard/steps/device_step.dart` per Edge Cases (missing)
- [x] T092 Show partial-state recovery warning when USB unplug mid-flash fails in `apps/esp32-desktop-installer/lib/wizard/steps/flash_step.dart` per Edge Cases / contracts/flash-pipeline.md (partial)
- [x] T093 [P] Map common flash/USB errors to suggested fixes in flash UI/log panel under `apps/esp32-desktop-installer/lib/wizard/` per FR-011 (partial)
- [x] T094 Require selected serial port before Device step can proceed in `apps/esp32-desktop-installer/lib/wizard/installer_session_controller.dart` per FR-001 (partial)
- [x] T095 [P] Replace bullet-list display layout preview with field-arrangement preview in `apps/esp32-desktop-installer/lib/wizard/steps/display_step.dart` per FR-005 (partial)
- [x] T096 [P] Apply advanced custom sensor JSON driver to humidity (not only temp parameters) in `apps/esp32-desktop-installer/lib/wizard/steps/sensors_step.dart` per FR-003 (partial)
- [x] T097 [P] When custom profile shadows a builtin id, hide or replace the builtin entry in `apps/esp32-desktop-installer/lib/wizard/steps/profiles_step.dart` per FR-006 (partial)
