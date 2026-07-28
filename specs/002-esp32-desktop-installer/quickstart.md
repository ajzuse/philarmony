# Quickstart: ESP32 Desktop Installer (Flutter)

**Feature**: `002-esp32-desktop-installer`  
**Prerequisites**: Flutter 3.22+ with desktop enabled, PlatformIO (firmware `001`), USB + ESP32. For **public macOS** packages: Apple Developer ID + notarization. Win/Linux packagers for MSIX/AppImage/deb/rpm (unsigned OK in MVP with docs warnings).

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
# Windows → .msix (unsigned OK MVP)
flutter build windows --release && dart run msix:create

# macOS → .dmg (public: codesign + notarize REQUIRED)
flutter build macos --release
# packaging/macos → notarized DMG

# Linux → AppImage + .deb + .rpm
make package-installer-linux
```

See `contracts/desktop-distribution.md`.

## 4. Validation scenarios

### VS-1 First-time device setup (hardware)
1. Connect ESP32 → complete wizard → Install.
2. Expect esptool stages; **Flash Success** when esptool verify OK.
3. If WS/hotspot unreachable → **warning**, not failure.
4. Device boots to WiFi or hotspot `philarmony`.

### VS-2 Pin conflict
Duplicate GPIOs → Next blocked; no erase.

### VS-3 Profile export/import
Export JSON (password `***`/omit) → import → re-enter password → flash.

### VS-4 Host installer smoke (Win / macOS)
1. Install MSIX or **notarized** DMG on clean machine.
2. App launches; USB ports enumerate.

### VS-5 Host installer smoke (Linux)
AppImage / `apt install ./…deb` / `dnf install ./…rpm` → launch → ports enumerate.

### VS-6 Flash failure Retry
Simulate flash fail (unplug mid-write or mock) → UI offers **Retry** (full reflash) → **no** claim of restored prior image.

### VS-7 Reconfigure from local only
Complete one flash → quit → relaunch → **Load last session** (no USB NVS read) → change WiFi → reflash.

### VS-8 Automated (CI / no hardware)

```bash
cd packages/philarmony_core && dart test
cd apps/esp32-desktop-installer && flutter test
make package-installer-linux   # Linux CI
```

## 5. Contracts

- `contracts/installer-profile.schema.json`
- `contracts/nvs-config-mapping.md`
- `contracts/flash-pipeline.md`
- `contracts/desktop-distribution.md`

## 6. Shared with 003

`packages/philarmony_core` shared; packaging patterns reusable for control app desktop builds.
