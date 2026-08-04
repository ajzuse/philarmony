# Implementation Plan: ESP32 Desktop Installer

**Branch**: `feature/002-esp32-desktop-installer` | **Date**: 2026-07-28 | **Spec**: `specs/002-esp32-desktop-installer/spec.md`  
**Status**: Implemented (2026-08-04) — product tasks closed; host/HW manual stages tracked in `005-manual-validation`

**Input**: Feature specification from `/specs/002-esp32-desktop-installer/spec.md`.

**Depends on**: Firmware feature `001-filament-dryer-esp32` (NVS/hardware JSON schema, optional post-flash WS/HTTP when reachable, pre-built PlatformIO binaries).

**Stack (2026-07-28):** **Flutter** shared with `003-filament-dryer-control-app` (DEC-010).  
**Distribution (2026-07-28):** Ship **OS-native installers** — Windows, macOS, Linux (DEC-011).  
**Amended**: Linux first-class (`make` + AppImage/deb/rpm).  
**Amended (clarify 2026-07-28):** Soft post-flash network verify; local-only reconfigure; password redaction; Retry without restore; macOS notarize required / Win+Linux unsigned MVP OK.

## Summary

Build a cross-platform **Flutter** desktop app that (1) guides ESP32 first-time setup and USB firmware flash, and (2) is itself delivered as **platform installers**. Domain logic in `packages/philarmony_core`. USB via `flutter_libserialport`; device flash via bundled **esptool**. **Flash Success** = esptool verify; WS/HTTP check is optional and non-blocking. Reconfigure prefills from **local last-session / JSON only** (no on-device NVS dump). Host packaging: **MSIX** (Windows, unsigned OK MVP), **notarized DMG** (macOS public), **AppImage + deb + rpm** via **Makefile** (unsigned OK MVP with docs warnings).

## Technical Context

**Language/Version**: Dart 3.x / Flutter 3.22+ (desktop: Windows, macOS, Linux)

**Primary Dependencies**:
- Flutter Desktop UI + `flutter_localizations` (PT-BR / EN-US)
- `flutter_libserialport` — USB-UART enumeration
- Bundled `esptool` per OS — erase/write/verify via `Process`
- `packages/philarmony_core` — models, validators, NVS mapping (shared with `003`)
- **Packaging**: `msix` (Windows); `flutter_distributor` / `create-dmg` (macOS DMG); Linux **AppImage + deb + rpm** via `flutter_distributor` / fpm / rpmbuild, wrapped by **Makefile**
- Code signing: **macOS Developer ID + notarization REQUIRED for public DMG**; Windows Authenticode optional in MVP (document SmartScreen); Linux GPG optional in MVP
- Post-flash: optional WS/HTTP when IP/hotspot reachable — MUST NOT fail Flash Success after esptool verify

**Storage**:
- Local JSON configuration profiles (export/import; password **always** omitted/`***`)
- Last successful DeviceProfile (sans password) in app support dir for reconfigure
- Bundled firmware + esptool inside the app bundle / installer payload
- Device NVS at flash time only (no USB NVS readback in this phase)

**Testing**: `flutter_test` + mocked flasher; CI builds Win/macOS/Linux packages; smoke installers; assert soft network-verify warning path

**Target Platform**:
- **Windows** 10/11 x64 — **MSIX** (optional Inno `.exe`); unsigned public MVP allowed with docs warning
- **macOS** 12+ — **DMG**; **public release MUST be signed + notarized**
- **Linux** x64 — AppImage + `.deb` + `.rpm`; unsigned MVP OK with docs; builds via `make package-installer-*`
- Mobile deferred to `003`

**Project Type**: Desktop application + release packaging pipeline

**Performance Goals**:
- USB detect &lt;2s; device flash ≤4MB &lt;30s; UI &lt;100ms
- Host installer size: target &lt;150MB compressed download including firmware assets

**Constraints**:
- End users MUST NOT need Flutter SDK, Visual Studio, or Xcode to install the app
- One download artifact per OS/format
- Linux packaging entry point for builders/CI: **Makefile**
- No firmware compile-from-source; USB flash only
- Pin safety validation before device erase
- Flash failure: **Retry** full pipeline — **no** automatic pre-erase image restore
- Reconfigure: local store / JSON import only — **no** on-device NVS dump over USB
- GPLv3 source; release binaries may need third-party notices for esptool
- WiFi passwords never in exported profiles (omit/`***` only — no “encrypted in JSON”)
- Unsigned macOS only for internal CI — **public** macOS MUST be notarized

**Scale/Scope**: Wizard + device flash + host installers Win/macOS/Linux; shared Dart core with `003`

## Constitution Check

*GATE: Must pass before Phase 0. Re-check after Phase 1.*

- [x] **Orientação a Objetos / Segurança de Hardware**: Domain classes encapsulate serial/flash; invalid pins abort.
- [x] **Desempenho**: Flash/serial off UI isolate; packaging is build-time only.
- [x] **Failsafe**: No device flash without pin validation; Flash Success = esptool verify; network check soft; Retry without false “rollback restore”.
- [x] **Workspace**: Flutter SDK + packaging tools documented in `.vscode/workspace.json`.
- [x] **Teste Automatizado**: Flow tests + CI package jobs; soft-verify path covered.
- [x] **Documentação Sincronizada**: README + installer docs (signing warnings, `dnf`/`apt`/AppImage).
- [x] **Memória Compartilhada**: DEC-010 + DEC-011; clarify session encoded in spec.
- [x] **Commit approval**: unchanged.

**Post-design (clarify amend)**: Gates OK. Clarifications remove NVS-read and auto-restore scope; tighten signing/MVP and Flash Success definition.

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
│   ├── flash-pipeline.md
│   └── desktop-distribution.md
└── tasks.md
```

### Source Code (repository root)

```text
packages/
└── philarmony_core/

apps/
└── esp32-desktop-installer/          # Flutter desktop app
    ├── lib/ …                        # wizard, device, flash
    ├── assets/
    │   ├── firmware/                 # 001 binaries
    │   └── tools/                    # esptool per-OS
    ├── windows/ + macos/ + linux/    # runner projects
    ├── dist/                         # generated installers (gitignored)
    ├── pubspec.yaml                  # msix_config, etc.
    └── packaging/
        ├── windows/msix.yaml         # or pubspec msix_config
        ├── macos/dmg.json            # create-dmg / distributor config
        ├── linux/
        │   ├── appimage.yml
        │   ├── deb/                  # control metadata
        │   └── rpm/                  # .spec / nfpm config
        └── README.md                 # how to release

Makefile (repo root, extend alongside firmware targets)
├── package-installer / package-installer-linux
├── package-installer-linux-appimage
├── package-installer-linux-deb
└── package-installer-linux-rpm

.github/workflows/
└── desktop-installer-release.yml     # attach MSIX/DMG/AppImage/deb/rpm to GitHub Release
```

**Structure Decision**: Flutter app + **Makefile-driven** Linux packaging + CI release pipeline produce downloadable installers per OS/format. Same app binary embeds firmware/esptool so users need no extra tools after installing Philarmony Desktop Installer.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|--------------------------------------|
| Flutter + bundled esptool | Reliable Espressif flash | Pure-Dart flash protocol too risky |
| Shared `philarmony_core` | Reuse with `003` | Duplicated schemas drift |
| OS installers (MSIX/DMG/AppImage/deb/rpm) | Single-download UX (user req.) | Zipping `build/…/Release` fails non-dev users / Gatekeeper |
| Code signing / notarization | macOS Gatekeeper + Win trust | Unsigned public builds blocked or scary warnings |
| Make + three Linux formats | DIY/Fedora + Debian + portable | Single AppImage alone misses native package managers |

**Constitution compliance:** Philarmony; bilingual install docs required.
