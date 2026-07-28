# Quickstart: ESP32 Desktop Installer (Flutter)

**Feature**: `002-esp32-desktop-installer`  
**Prerequisites**: Flutter 3.22+ with desktop enabled, PlatformIO (to produce `001` firmware artifacts), USB cable + ESP32 board.

## 1. Produce firmware artifacts (from feature 001)

```bash
make build ENV=esp32devkitc
# Copy build outputs into apps/esp32-desktop-installer/assets/firmware/
```

## 2. Run installer (dev)

```bash
cd apps/esp32-desktop-installer
flutter pub get
flutter run -d windows   # or macos / linux
```

Expected: wizard opens; locale PT-BR/EN; empty device list shows troubleshooting when no USB UART present.

## 3. Validation scenarios

### VS-1 First-time setup (hardware)
1. Connect ESP32 via USB.
2. Complete wizard → Install.
3. Expect esptool-driven stages to succeed (&lt;30s typical).
4. Device boots to WiFi or hotspot `philarmony`.

### VS-2 Pin conflict
Assign duplicate GPIOs → Next blocked; no erase.

### VS-3 Profile export/import
Export JSON (password placeholder) → import → re-enter password → flash.

### VS-4 Automated (CI / no hardware)

```bash
cd packages/philarmony_core && dart test
cd apps/esp32-desktop-installer && flutter test
```

## 4. Contracts

- `contracts/installer-profile.schema.json`
- `contracts/nvs-config-mapping.md`
- `contracts/flash-pipeline.md`
- Firmware verify: `specs/001-filament-dryer-esp32/contracts/http-api.md`

## 5. Shared with 003

Domain models/validators live in `packages/philarmony_core` — control app must depend on the same package (no duplicated profile schema).
