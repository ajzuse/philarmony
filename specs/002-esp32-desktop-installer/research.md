# Research: ESP32 Desktop Installer

**Feature**: `002-esp32-desktop-installer` | **Date**: 2026-07-28  
**Amended**: 2026-07-28 — stack switched from Tauri to Flutter per product direction (shared codebase with `003`).  
**Amended**: 2026-07-28 — host OS installers (MSIX / DMG / Linux AppImage+deb+rpm via Make) (DEC-011).

## R1 — Desktop / shared UI framework

**Decision**: **Flutter 3.x (Dart)** for the installer, with shared packages for feature `003` (control app desktop + mobile).

**Rationale**:
- Spec `003` already targets Flutter for Win/macOS/Linux + Android/iOS.
- User priority: one Dart codebase evolving from installer → dryer control UI.
- Flutter desktop is production-ready on the three desktop OSes required by `002`.
- Wizard UX, i18n (ARB), and theming map cleanly to Material.

**Does Flutter support everything needed?**

| Need (FR) | Flutter support | Approach |
|-----------|-----------------|----------|
| Multi-step wizard UI | Yes | Navigator / go_router + form state |
| PT-BR / EN-US | Yes | `flutter_localizations` + ARB |
| USB device list (CP210x/CH340/FTDI) | Yes (desktop) | `flutter_libserialport` / libserialport |
| Chip detect + flash erase/write/verify | Indirect | Bundle **esptool** and drive via `Process` (parse stdout for progress) |
| Config JSON profiles | Yes | `dart:convert` + shared models |
| Post-flash network verify | Yes | HTTP/`web_socket_channel` (same stack as `003`) |
| Win/macOS/Linux packages | Yes | `flutter build` + MSIX/DMG + Linux AppImage/deb/**rpm** via Make (R7) |
| USB flash on iOS/Android | Partial / out of MVP | Desktop installer owns flash; mobile control app uses WiFi/WS |

**Alternatives considered**:
- **Tauri 2 + React** (prior DEC): Smaller binary, excellent Rust `espflash` — rejected to maximize sharing with `003`.
- **Electron**: Heavy; weaker alignment with Flutter roadmap.
- **Pure Dart flash protocol**: High effort/risk vs mature esptool.

## R2 — Flashing stack

**Decision**: Ship platform-specific **esptool** binaries under `assets/tools/` and orchestrate from Dart (`FirmwareFlasher`). Use `flutter_libserialport` for port enumeration and optional serial logs; hand the same port path to esptool for flash.

**Rationale**: Espressif flash protocol (stub, flash mode, compression, verify) is non-trivial; esptool is the supported tool. Flutter’s job is UX + validation + packaging + progress.

**Alternatives considered**:
- FFI to `espflash` Rust crate: Possible later optimization; not required for MVP.
- Shell to system Python esptool: Fragile on end-user machines — prefer bundled binary.

## R3 — Config injection (NVS coupling)

**Decision**: Unchanged — map installer profile → firmware NVS/JSON (`contracts/nvs-config-mapping.md`). Shared `philarmony_core` owns the mapper so `003` cannot drift.

## R4 — Device detection

**Decision**: `SerialPort.availablePorts` + VID/PID filtering where available; chip/flash size via esptool chip_id / flash_id after port select.

## R5 — Shared codebase layout with `003`

**Decision**:
- `packages/philarmony_core` — models, validation, profile↔firmware mapping, i18n keys where domain-level
- `apps/esp32-desktop-installer` — wizard + serial + flasher (002)
- `apps/filament-dryer-control` — later (003), depends on core (+ WS device client)

Installer remains **desktop-first** because USB flashing is the hard requirement; control app adds mobile without needing flash on phone for MVP.

## R6 — Testing strategy

**Decision**: Unit-test validators/mappers without hardware; mock `Flasher`/`SerialPort` interfaces for flow tests; one manual hardware smoke path in quickstart; CI runs `flutter test` on Linux (and macOS/Windows as runners allow).

## R7 — Host app distribution (single-download installers)

**Decision**: Ship **OS-native installers** so non-developers install with one download:

| OS | Artifact | Tooling |
|----|----------|---------|
| Windows 10/11 x64 | **`.msix`** (primary) | `msix` pub package (`dart run msix:create`); optional Inno Setup `.exe` fallback if MSIX sideload policy blocks some orgs |
| macOS 12+ | **`.dmg`** (drag-to-Applications) | `flutter build macos` → sign (Developer ID) → notarize → `create-dmg` / `flutter_distributor` |
| Linux x64 | **AppImage** + **`.deb`** + **`.rpm`** | `flutter build linux` → package via `flutter_distributor` / `nfpm` / `fpm`; **entry point: `make package-installer-linux*`** |

**Linux formats**:
- **AppImage** — portable, no root; any distro
- **`.deb`** — Debian, Ubuntu, derivatives (`apt install ./…`)
- **`.rpm`** — Fedora, RHEL, Rocky, Alma, openSUSE-compatible rpm workflows (`dnf`/`yum`/`zypper` as applicable)

**Rationale**: User requirement for single-download multi-platform install including first-class Linux and Red Hat–based distros. Zipping Flutter `Release/` folders fails Gatekeeper and non-technical UX. Make aligns with existing firmware façade (DEC-009) for builders/CI without making clone+make the end-user path.

**Release pipeline**: GitHub Actions builds signed (when secrets present) or unsigned CI artifacts; GitHub Releases attach one file per OS/format. App payload **embeds** firmware + esptool so users need no second download for device flashing.

**Alternatives considered**:
- Portable ZIP only — rejected (Gatekeeper, no Start Menu, poor UX).
- Microsoft Store / Mac App Store only — optional later; sideload MSIX/DMG required for GPLv3 DIY audience.
- Separate “online installer” bootstrapper — unnecessary complexity for MVP.
- Linux AppImage-only — rejected; misses native package managers and Fedora/RHEL users.
- Flatpak/Snap as primary — deferred; AppImage + deb + rpm cover the requested surface.

## Open items deferred to tasks

- Exact esptool packaging (PyInstaller vs official standalone builds) per OS
- NVS image write format spike
- Apple notarization credentials + Windows code-signing cert in CI
- Whether to offer both MSIX and Inno `.exe` for Windows enterprise quirks
- Choose Linux packager (`nfpm` vs `fpm` vs `flutter_distributor`) and Makefile wiring details
- RPM metadata (Requires, desktop file, udev rules for serial if needed)
