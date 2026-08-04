# Desktop Distribution Contract — Host App Installers

**Scope**: How end users **install the Philarmony Desktop Installer application** (Flutter host), not how the ESP32 is flashed.

## Goals

1. **Single download** per OS/format — no Flutter SDK, no unzip-of-Release-folder.
2. **First-class**: Windows + macOS + **Linux** (AppImage, `.deb`, `.rpm`).
3. Installed app includes bundled firmware + esptool (no second download for device flash).
4. Publish via **GitHub Releases** with SHA-256 checksums.
5. **Linux build entry point**: repo **Makefile** targets (developers + CI); end users still get native packages.

## Artifacts

| OS | Format | Suggested asset name | Install UX |
|----|--------|----------------------|------------|
| Windows | `.msix` | `PhilarmonyInstaller-<ver>-windows-x64.msix` | Double-click / App Installer; Start Menu |
| Windows (fallback) | `.exe` (Inno) | `PhilarmonyInstaller-<ver>-windows-x64-setup.exe` | Classic wizard if MSIX sideload blocked |
| macOS | `.dmg` | `PhilarmonyInstaller-<ver>-macos-universal.dmg` | Mount → drag to Applications |
| Linux | `.AppImage` | `PhilarmonyInstaller-<ver>-linux-x64.AppImage` | `chmod +x` → run (portable) |
| Linux (Debian/Ubuntu) | `.deb` | `PhilarmonyInstaller-<ver>-linux-x64.deb` | `sudo apt install ./…deb` |
| Linux (Fedora/RHEL-family) | `.rpm` | `PhilarmonyInstaller-<ver>-linux-x64.rpm` | `sudo dnf install ./…rpm` (Fedora, RHEL, Rocky, Alma, …) |

## Makefile targets (Linux packaging)

Extend root `Makefile` (or `apps/esp32-desktop-installer/Makefile` included from root) so packaging is discoverable next to firmware targets:

| Target | Outcome |
|--------|---------|
| `make package-installer-linux` | Build all Linux formats (AppImage + deb + rpm) |
| `make package-installer-linux-appimage` | AppImage only |
| `make package-installer-linux-deb` | `.deb` only |
| `make package-installer-linux-rpm` | `.rpm` only (Red Hat family) |
| `make package-installer` | Alias / matrix helper (document OS=) |

Implementation note: targets wrap `flutter build linux --release` then packaging (e.g. `flutter_distributor`, `nfpm`, or `fpm`). Host deps for building rpm/deb documented in packaging README / workspace.json.

## Build commands (implement phase)

```bash
# Windows
flutter build windows --release
dart run msix:create

# macOS
flutter build macos --release
# codesign + notarytool (CI secrets)
# create-dmg / flutter_distributor → .dmg

# Linux (prefer Make)
make package-installer-linux              # AppImage + deb + rpm
make package-installer-linux-rpm          # Fedora / RHEL-family only
# equivalent manual:
# flutter build linux --release
# flutter_distributor / nfpm → AppImage, .deb, .rpm → apps/.../dist/
```

## Signing

| OS | Requirement for public release |
|----|--------------------------------|
| Windows | Authenticode **optional in MVP**; unsigned OK with SmartScreen warning in docs |
| macOS | **REQUIRED**: Developer ID Application + **Apple notarization** for any **public** GitHub Release DMG |
| Linux | GPG **optional in MVP**; unsigned AppImage/deb/rpm OK with trust-prompt docs |

Internal/CI may produce unsigned macOS builds for smoke only — not for public download pages.

## Release checklist

- [ ] Version bump matches `pubspec.yaml` / git tag `installer-vX.Y.Z`
- [ ] Firmware assets version recorded in release notes (`embeds_firmware_version`)
- [ ] SHA-256 for each asset published (incl. `.rpm` and `.deb`)
- [ ] Smoke: Win MSIX, macOS **notarized** DMG, Linux AppImage + `dnf install` rpm + `apt install` deb
- [ ] Docs warn if Win/Linux artifacts are unsigned (MVP policy)
- [ ] Docs PT-BR/EN-US “Download & Install” updated (Fedora/`dnf` called out)

## Non-goals (this phase)

- Auto-update channel (can use MSIX App Installer / Sparkle / Flatpak later)
- Mac App Store / Microsoft Store listing (optional follow-up)
- Flatpak/Snap as primary (AppImage + deb + rpm cover portable + Debian + Red Hat; Flatpak optional later)
- Online bootstrapper that downloads firmware separately
- Replacing end-user installers with “clone repo and `make`” — Make is for **building** packages, not the user install path
