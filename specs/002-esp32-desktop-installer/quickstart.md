# Quickstart: ESP32 Desktop Installer (Flutter)

**Feature**: `002-esp32-desktop-installer`  
**Prerequisites**: Flutter 3.22+ with desktop enabled, PlatformIO (firmware `001`), USB + ESP32. For **release packages**: Windows SDK (MSIX), Apple Developer ID (macOS notarization), Linux packagers for AppImage/deb/rpm (see packaging README).

## 1. Produce firmware artifacts (from feature 001)

```bash
make build ENV=esp32devkitc
# Copy build outputs into apps/esp32-desktop-installer/assets/firmware/
```

## 2. Run installer app (dev)

```bash
cd apps/esp32-desktop-installer
flutter pub get
flutter run -d windows   # or macos / linux
```

Expected: wizard opens; locale PT-BR/EN; empty device list shows troubleshooting when no USB UART present.

## 3. Build host OS installers (single download)

```bash
# Windows → one .msix
flutter build windows --release
dart run msix:create
# → dist/PhilarmonyInstaller-*-windows-x64.msix

# macOS → one .dmg (after codesign + notarize)
flutter build macos --release
# packaging/macos → create-dmg / flutter_distributor
# → dist/PhilarmonyInstaller-*-macos-*.dmg

# Linux → AppImage + .deb + .rpm (Makefile entry point)
make package-installer-linux
# or selectively:
make package-installer-linux-appimage
make package-installer-linux-deb
make package-installer-linux-rpm    # Fedora / RHEL / Rocky / Alma
# → dist/PhilarmonyInstaller-*-linux-x64.{AppImage,deb,rpm}
```

End-user path: download artifact → install → launch **Philarmony Installer** (no Flutter SDK).

See `contracts/desktop-distribution.md`.

## 4. Validation scenarios

### VS-1 First-time device setup (hardware)
1. Connect ESP32 via USB.
2. Complete wizard → Install (device flash).
3. Expect esptool stages &lt;30s typical.
4. Device boots to WiFi or hotspot `philarmony`.

### VS-2 Pin conflict
Duplicate GPIOs → Next blocked; no erase.

### VS-3 Profile export/import
Export JSON (password placeholder) → import → re-enter password → flash.

### VS-4 Host installer smoke (Win / macOS)
1. Install MSIX (Win) or DMG (macOS) on a clean machine.
2. App appears in Start Menu / Applications.
3. Launch succeeds; USB port list enumerates (with adapter plugged).

### VS-5 Host installer smoke (Linux)
1. **AppImage**: `chmod +x` → run → wizard opens.
2. **deb**: `sudo apt install ./PhilarmonyInstaller-*-linux-x64.deb` → desktop entry → launch.
3. **rpm** (Fedora or Rocky): `sudo dnf install ./PhilarmonyInstaller-*-linux-x64.rpm` → launch.
4. Serial ports enumerate with adapter plugged (udev/group notes in docs if needed).

### VS-6 Automated (CI / no hardware)

```bash
cd packages/philarmony_core && dart test
cd apps/esp32-desktop-installer && flutter test
make package-installer-linux   # on Linux CI runner (unsigned OK)
# CI also: flutter build windows|macos (+ package step when secrets allow)
```

## 5. Contracts

- `contracts/installer-profile.schema.json`
- `contracts/nvs-config-mapping.md`
- `contracts/flash-pipeline.md`
- `contracts/desktop-distribution.md`
- Firmware verify: `specs/001-filament-dryer-esp32/contracts/http-api.md`

## 6. Shared with 003

Domain models/validators live in `packages/philarmony_core` — control app must depend on the same package (no duplicated profile schema). Host packaging patterns (MSIX/DMG/Make+AppImage/deb/rpm) should be reusable for the control app desktop builds later.
