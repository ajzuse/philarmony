# Quickstart: Filament Dryer Control App (Flutter)

**Feature**: `003-filament-dryer-control-app`  
**Amended**: 2026-08-10 — Clarifications: CSV+PDF MVP; background WS required iOS+Android; Stop-only  
**Prerequisites**: Flutter 3.22+ (desktop + mobile); firmware `001` WS on LAN; optional mDNS.

## 1. Dev run (shared codebase)

```bash
cd apps/filament-dryer-control
flutter pub get
flutter run -d android   # or ios
flutter run -d macos     # or windows / linux
```

Expected: adaptive shell; PT-BR/EN; Devices = Scan + manual connect (`KnownDevice`).

## 2. Connect without mDNS

1. Host = ESP32 IP, port **80**, path `/ws` → Connect.  
2. `status/subscribe` → Dashboard from `status/update`.

## 3. Automated validation (CI / no hardware)

```bash
cd packages/philarmony_core && dart test
cd apps/filament-dryer-control && flutter test && flutter analyze
```

Mock `PhilarmonyWsClient` only in CI.

## 4. Validation scenarios

### VS-1 Discovery & connect
Scan or manual → Connected → first status &lt;2s.

### VS-2 Start / stop (no pause)
Start (profile or explicit) → `drying` → **Stop** with confirm → heater/fan off. UI MUST NOT offer Pause/Resume.

### VS-3 Config hardware
Pin conflict blocked locally → ack / error topics per `001`.

### VS-4 History offline
Completed cycle → offline → list + detail from SQLite.

### VS-5 Adaptive layout
Resize across 600 / 1024 → chrome per `contracts/adaptive-layout.md`.

### VS-6 Multi-device
Two `KnownDevice`s → switcher updates session; max concurrent respected.

### VS-7 History export CSV + PDF
Export same completed cycle as CSV **and** PDF → both openable; PDF has summary + charts (`contracts/history-export.md`).

### VS-8 Mobile background WS
Start cycle → background app ≥2 min (Android + iOS) → session still receives status or reconnects per policy; notification on complete/fault when permitted.

### VS-9 Packaging smoke (desktop)
`make package-control-linux` (etc.) → launch (manual → `005-manual-validation`).

## 5. Contracts

- `contracts/ws-client.md`
- `contracts/adaptive-layout.md`
- `contracts/package-structure.md`
- `contracts/history-export.md`
- Firmware: `specs/001-filament-dryer-esp32/contracts/websocket-api.md`

## 6. Manual catalog

Open hardware/UI/background-device smoke → `specs/005-manual-validation` only.
