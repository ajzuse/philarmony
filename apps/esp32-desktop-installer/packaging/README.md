# Packaging — Philarmony Desktop Installer

## Flutter via FVM (required)

Project is pinned to **Flutter 3.44.0** (see `.fvmrc` / `.fvm/fvm_config.json`).

```bash
# once per machine
brew install fvm   # or https://fvm.app
cd /path/to/secadora
fvm install 3.44.0
fvm use 3.44.0

# always use fvm wrappers (Makefile does this automatically when fvm is on PATH)
fvm flutter --version
fvm dart --version

# or via Make shortcuts
make run                 # DEVICE defaults to host OS (macos/linux/windows)
make run-macos
make devices
make doctor
make pub-get
make test-flutter
make create-platforms    # once: generate windows/macos/linux runners
```

VS Code / Cursor: `dart.flutterSdkPath` → `.fvm/flutter_sdk` (in `.vscode/settings.json`).

## Prerequisites

1. FVM + Flutter 3.44.0 as above; enable desktop: `fvm flutter config --enable-macos-desktop` (etc.)
2. From this app dir: `fvm flutter create --platforms=windows,macos,linux .`
3. Bundle esptool under `assets/tools/{os}/` and firmware under `assets/firmware/`

## Host artifacts (DEC-011)

| OS | Command | Public signing |
|----|---------|----------------|
| Windows | `fvm flutter build windows --release && fvm dart run msix:create` | Unsigned OK in MVP (document SmartScreen) |
| macOS | `fvm flutter build macos --release` → sign + **notarize** → DMG | **REQUIRED** for public release |
| Linux | `make package-installer-linux` from repo root | Unsigned OK in MVP with docs warning |

### Linux Make targets

- `make package-installer-linux` — AppImage + deb + rpm
- `make package-installer-linux-appimage`
- `make package-installer-linux-deb`
- `make package-installer-linux-rpm`

Artifacts land in `apps/esp32-desktop-installer/dist/` (gitignored).

### Windows MSIX naming

After `make package-installer-windows` / `fvm dart run msix:create`:

- Default output under `apps/esp32-desktop-installer/build/windows/` or app root as
  `philarmony_installer.msix` / `esp32_desktop_installer.msix` (msix package identity `local.philarmony.installer`)
- Copy/rename release artifact to `dist/PhilarmonyInstaller-0.1.0.msix` for GitHub Releases
- Unsigned MVP: SmartScreen may warn — documented in `docs/EN-US/installer.md`

Optional: Inno Setup script at `packaging/windows/inno/philarmony-installer.iss` → `dist/PhilarmonyInstaller-Setup-0.1.0.exe`

## Bundle esptool

```bash
make bundle-esptool          # host OS only
# or
./scripts/bundle-esptool.sh macos|linux|windows
```

Writes into `assets/tools/{os}/esptool[.exe]` and updates `packaging/ESPTOOL_CHECKSUMS.txt`.
Archive SHA256 values are verified against Espressif v5.3.0 release notes before extract.
Binaries are gitignored (large); CI/release jobs must run `make bundle-esptool` before packaging.

Firmware: `make sync-installer-firmware` from repo root.
