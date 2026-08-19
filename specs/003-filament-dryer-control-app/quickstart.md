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

## 4. Validation scenarios (manual → 005)

Narrative acceptance only. **Open manual stages** live in `specs/005-manual-validation/tasks.md` as **VS-CTRL-1…VS-CTRL-9**. Record results in `checklists/quickstart-validation.md`.

| 003 narrative | 005 stage |
|---------------|-----------|
| Discovery & connect | **VS-CTRL-1** |
| Start / stop (no pause) | **VS-CTRL-2** |
| Config hardware | **VS-CTRL-3** |
| History offline | **VS-CTRL-4** |
| Adaptive layout | **VS-CTRL-5** |
| Multi-device | **VS-CTRL-6** |
| History export CSV + PDF | **VS-CTRL-7** |
| Mobile background WS | **VS-CTRL-8** |
| Packaging smoke (desktop) | **VS-CTRL-9** |

### Scenario summaries

- **VS-CTRL-1**: Scan or manual → Connected → first status &lt;2s.
- **VS-CTRL-2**: Start → `drying` → **Stop** with confirm; UI MUST NOT offer Pause/Resume.
- **VS-CTRL-3**: Pin conflict blocked locally → ack / error topics per `001`.
- **VS-CTRL-4**: Completed cycle → offline → list + detail from local store.
- **VS-CTRL-5**: Resize across 600 / 1024 → chrome per `contracts/adaptive-layout.md`.
- **VS-CTRL-6**: Two `KnownDevice`s → switcher; max concurrent respected.
- **VS-CTRL-7**: Export same cycle as CSV **and** PDF → both openable (`contracts/history-export.md`).
- **VS-CTRL-8**: Active cycle → background ≥2 min (Android + iOS) → status/reconnect + notification policy.
- **VS-CTRL-9**: `make package-control-linux` (etc.) → launch.

## 5. Contracts

- `contracts/ws-client.md`
- `contracts/adaptive-layout.md`
- `contracts/package-structure.md`
- `contracts/history-export.md`
- Firmware: `specs/001-filament-dryer-esp32/contracts/websocket-api.md`

## 6. Manual catalog

All open hardware/UI/background/packaging smoke for this feature → `specs/005-manual-validation` only (stages **VS-CTRL-***). No open `[MANUAL]` tasks on this product `tasks.md`.

