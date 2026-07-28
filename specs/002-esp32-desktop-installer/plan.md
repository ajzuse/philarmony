# Implementation Plan: ESP32 Desktop Installer

**Branch**: `feature/002-esp32-desktop-installer` | **Date**: 2026-07-28 | **Spec**: `specs/002-esp32-desktop-installer/spec.md`

**Input**: Feature specification from `/specs/002-esp32-desktop-installer/spec.md`.

**Depends on**: Firmware feature `001-filament-dryer-esp32` (NVS/hardware JSON schema, WebSocket verify after flash, pre-built PlatformIO binaries).

**Stack revision (2026-07-28):** User requirement — **Flutter** (not Tauri) so the installer shares Dart packages with feature `003-filament-dryer-control-app` (desktop + mobile control of the dryer). DEC-010 amended accordingly.

## Summary

Build a cross-platform **Flutter** desktop installer that guides users through ESP32 model, sensors, GPIO mapping, display, filament profiles, and WiFi, then flashes a pre-built Philarmony firmware image with an injected NVS/config payload over USB. Domain logic (profiles, pin validation, NVS mapping) lives in shared Dart packages reused by the future control app. USB port discovery uses `flutter_libserialport`; flashing invokes a **bundled `esptool`** (or equivalent) via Dart `Process` with progress parsing. Config output stays compatible with firmware `001` `ConfigManager` / `HardwareConfigParser`.

## Technical Context

**Language/Version**: Dart 3.x / Flutter 3.22+ (stable desktop: Windows, macOS, Linux)

**Primary Dependencies**:
- Flutter Desktop (Material / Cupertino as needed)
- `flutter_libserialport` — enumerate/open USB-UART ports (Win/macOS/Linux; Android later if needed)
- Bundled `esptool` (PyInstaller/standalone or `esptool` CLI shipped per OS) invoked via `Process.start` for erase/write/verify
- `flutter_localizations` + ARB (PT-BR / EN-US)
- Shared package(s): `packages/philarmony_core` (models, validators, profile JSON ↔ firmware map)
- Optional later: `web_socket_channel` in shared device package for post-flash verify / control app

**Storage**:
- Local JSON configuration profiles (export/import; password stripped)
- Bundled firmware under `apps/esp32-desktop-installer/assets/firmware/`
- Device NVS written at flash time

**Testing**: `flutter_test` unit tests (validators, mapping); integration/flow tests with mocked `Flasher` / serial; CI matrix desktop smoke builds

**Target Platform**: Windows 10/11 x64, macOS 12+ (Intel/Apple Silicon), Linux Ubuntu 20.04+ (primary). Mobile shells deferred to `003` (installer remains desktop-first for USB flash).

**Project Type**: Desktop application (installer / flasher) in a Flutter monorepo shared with future control app

**Performance Goals**:
- USB device detect &lt;2s
- Full flash (≤4MB app) &lt;30s typical
- UI interaction latency &lt;100ms (isolate/async for flash I/O)

**Constraints**:
- No compile-from-source in this phase (pre-built firmware only)
- USB flash only (no OTA) — requires desktop (or Android USB-OTG later; not MVP)
- Pin/safety validation MUST reject conflicts and input-only GPIOs (align with firmware FR-003)
- GPLv3 for installer source; firmware binaries remain GPLv3
- WiFi password: never write plaintext to exported profiles
- Flash path depends on shipping working `esptool` binaries per OS (documented packaging task)

**Scale/Scope**: Single-device wizard; profile reuse for sequential bulk flash; one firmware channel matching `001`; shared Dart core with `003`

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Orientação a Objetos e Segurança de Hardware**: Dart classes (`DeviceDetector`, `PinValidator`, `NvsConfigBuilder`, `FirmwareFlasher`) encapsulate USB/flash; invalid pins abort before erase/write.
- [x] **Desempenho Máximo e Eficiência**: Host-side — flash/serial off UI isolate; no blocking UI. ESP32 RAM gates apply to bundled firmware only.
- [x] **Failsafe e Proteção de Hardware**: Wizard validates GPIO/PWM capability and conflicts; verify checksum; never flash when validation fails.
- [x] **Workspace de Dependências e Configurações**: Flutter SDK + Dart in `.vscode/workspace.json`; `pubspec.lock` committed; document esptool asset layout.
- [x] **Teste Automatizado e Qualidade**: Flow tests for wizard + pin-conflict + profile I/O; CI fails on test failure.
- [x] **Documentação Sincronizada (README Vivo)**: after_plan hook; docs EN-US/PT-BR installer pages.
- [x] **Memória Compartilhada**: DEC-010 amended (Flutter shared codebase).
- [x] **Revisão e Aprovação Explícita de Commit**: Unchanged; no auto-commit.

**Post-design re-check**: Gates satisfied. Flutter covers detection, wizard UX, i18n, and flash orchestration; native flash protocol remains esptool (industry standard), not reimplemented in Dart.

## Project Structure

### Documentation (this feature)

```text
specs/002-esp32-desktop-installer/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── installer-profile.schema.json
│   ├── nvs-config-mapping.md
│   └── flash-pipeline.md
└── tasks.md            # /speckit-tasks (not created here)
```

### Source Code (repository root)

```text
packages/
└── philarmony_core/              # Shared with 003
    ├── lib/src/models/           # DeviceProfile, pins, sensors, WiFi…
    ├── lib/src/validation/       # PinValidator, profile schema
    └── lib/src/mapping/          # Installer profile → firmware NVS/JSON

apps/
└── esp32-desktop-installer/      # Flutter desktop app (002)
    ├── lib/
    │   ├── app/
    │   ├── features/wizard/
    │   ├── features/profiles/
    │   ├── features/flash/       # Process→esptool + progress parse
    │   └── features/device/      # flutter_libserialport wrapper
    ├── assets/
    │   ├── firmware/             # Pre-built 001 binaries
    │   └── tools/                # esptool per-OS binaries
    ├── test/
    └── integration_test/
```

**Future (`003`)**: `apps/filament-dryer-control/` depends on `philarmony_core` (+ device WebSocket package); reuses models/validators; no USB flash required on mobile.

**Structure Decision**: Flutter monorepo maximizes reuse with the control app. Installer stays desktop-first because USB flashing is the constraint; shared packages carry domain logic forward.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|--------------------------------------|
| Flutter + bundled esptool | Spec needs reliable Espressif flash protocol | Pure-Dart flash reimplementation is high-risk / incomplete |
| Shared package layer early | User wants codebase shared with `003` | Duplicating models across apps drifts NVS contracts |
| Desktop-only installer MVP | USB-UART flash | Mobile USB-OTG flash deferred (possible later, not required by `002` AC) |

**Constitution compliance:** PROJECT_NAME = Philarmony; pin safety + bilingual docs preserved.
