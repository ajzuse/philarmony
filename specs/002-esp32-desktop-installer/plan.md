# Implementation Plan: ESP32 Desktop Installer

**Branch**: `feature/002-esp32-desktop-installer` | **Date**: 2026-07-28 | **Spec**: `specs/002-esp32-desktop-installer/spec.md`

**Input**: Feature specification from `/specs/002-esp32-desktop-installer/spec.md`.

**Depends on**: Firmware feature `001-filament-dryer-esp32` (NVS/hardware JSON schema, WebSocket verify after flash, pre-built PlatformIO binaries).

**Stack (2026-07-28):** **Flutter** shared with `003-filament-dryer-control-app` (DEC-010).  
**Distribution (2026-07-28):** Ship **OS-native installers** for single-download install — **Windows**, **macOS**, and **Linux** (DEC-011).  
**Amended**: Linux elevated to first-class: **`make` package targets** + AppImage + `.deb` + **`.rpm`** (Fedora / RHEL-family).

## Summary

Build a cross-platform **Flutter** desktop app that (1) guides ESP32 first-time setup and USB firmware flash, and (2) is itself delivered as **platform installers** so end users download a single file and install without unzipping Flutter build folders. Domain logic lives in shared Dart packages for the future control app. USB via `flutter_libserialport`; device flash via bundled **esptool**. Host packaging: **MSIX** (Windows), **signed+notarized DMG** (macOS), and on Linux **AppImage + `.deb` + `.rpm`**, driven primarily through **repo `Makefile` targets** (plus CI).

## Technical Context

**Language/Version**: Dart 3.x / Flutter 3.22+ (desktop: Windows, macOS, Linux)

**Primary Dependencies**:
- Flutter Desktop UI + `flutter_localizations` (PT-BR / EN-US)
- `flutter_libserialport` — USB-UART enumeration
- Bundled `esptool` per OS — erase/write/verify via `Process`
- `packages/philarmony_core` — models, validators, NVS mapping (shared with `003`)
- **Packaging**: `msix` (Windows); `flutter_distributor` / `create-dmg` (macOS DMG); Linux **AppImage + deb + rpm** via `flutter_distributor` / fpm / rpmbuild, wrapped by **Makefile**
- Code signing: Windows Authenticode (optional for sideload MVP; required for Store); Apple Developer ID + notarization (required for Gatekeeper); Linux optional GPG on packages

**Storage**:
- Local JSON configuration profiles (export/import; password stripped)
- Bundled firmware + esptool inside the app bundle / installer payload
- Device NVS at flash time

**Testing**: `flutter_test` + mocked flasher; CI builds Win/macOS/Linux packages (signing secrets when available); smoke “installer launches” incl. rpm/deb install on VMs

**Target Platform**:
- **Windows** 10/11 x64 — **MSIX** (optional Inno `.exe`)
- **macOS** 12+ — **DMG** (signed + notarized for public release)
- **Linux** x64 — **AppImage** (portable), **`.deb`** (Debian/Ubuntu), **`.rpm`** (Fedora, RHEL, Rocky, Alma, etc.); built via `make package-installer-*`
- Mobile deferred to `003` (no USB flash requirement there)

**Project Type**: Desktop application + release packaging pipeline

**Performance Goals**:
- USB detect &lt;2s; device flash ≤4MB &lt;30s; UI &lt;100ms
- Host installer size: target &lt;150MB compressed download including firmware assets (monitor; document if exceeded)

**Constraints**:
- End users MUST NOT need Flutter SDK, Visual Studio, or Xcode to install the app
- One download artifact **per OS/format** (Win x64; macOS universal or dual; Linux AppImage + deb + rpm)
- Linux packaging entry point for developers/CI: **Makefile** (does not replace end-user `.rpm`/`.deb`/AppImage downloads)
- No firmware compile-from-source; USB flash only
- Pin safety validation before device erase
- GPLv3 source; release binaries may need third-party notices for esptool
- WiFi passwords never in exported profiles
- Unsigned macOS builds only for internal CI — public releases require notarization

**Scale/Scope**: Wizard + device flash + **host app installers** for Win/macOS/Linux; shared Dart core with `003`

## Constitution Check

*GATE: Must pass before Phase 0. Re-check after Phase 1.*

- [x] **Orientação a Objetos / Segurança de Hardware**: Domain classes encapsulate serial/flash; invalid pins abort.
- [x] **Desempenho**: Flash/serial off UI isolate; packaging is build-time only.
- [x] **Failsafe**: No device flash without validation; host installer must not leave half-installed state (use OS installer semantics — MSIX/DMG/deb/rpm).
- [x] **Workspace**: Flutter SDK + packaging tools (incl. rpm/deb/AppImage) documented in `.vscode/workspace.json`.
- [x] **Teste Automatizado**: Flow tests + CI package build job (at least unsigned artifact generation for Win/macOS/Linux formats).
- [x] **Documentação Sincronizada**: README + `docs/*/installer*` describe download/install steps (incl. `dnf`/`apt`/AppImage).
- [x] **Memória Compartilhada**: DEC-010 (Flutter) + DEC-011 (host distribution, amended Linux).
- [x] **Commit approval**: unchanged.

**Post-design**: Gates OK. Linux Make + rpm/deb/AppImage are additive packaging; firmware flash safety unchanged.

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
