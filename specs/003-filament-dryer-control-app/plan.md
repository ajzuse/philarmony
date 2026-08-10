# Implementation Plan: Filament Dryer Control App

**Branch**: `feature/003-filament-dryer-control-app` | **Date**: 2026-08-10 | **Spec**: `specs/003-filament-dryer-control-app/spec.md`  
**Amended**: 2026-08-10 — sync Clarifications Session (WS=`001`, `KnownDevice`, CSV+PDF MVP, background WS iOS+Android required, Stop-only)

**Input**: Feature specification from `/specs/003-filament-dryer-control-app/spec.md`.

**Depends on**: Firmware `001-filament-dryer-esp32` (WebSocket API contract), shared Dart core from `002-esp32-desktop-installer` (`packages/philarmony_core`, DEC-010).

**Stack**: **Flutter** single codebase — Desktop (Win/macOS/Linux) + Mobile (Android/iOS) + tablet layouts via adaptive breakpoints. Maximize shared UI/domain; platform channels only where required (mDNS, notifications, **required** background WS).

## Summary

Build a cross-platform **Flutter** control app that discovers Philarmony dryers on the LAN, connects via WebSocket to the firmware API (`001` contracts — normative), and provides real-time dashboard, cycle **start/stop** (no pause), hardware/display config, material profiles, multi-device `KnownDevice` registry, and local history with **CSV + PDF** export. Domain in `packages/philarmony_core`; UI in `apps/filament-dryer-control` (one `lib/` for all targets). Adaptive phone/tablet/desktop chrome. Mobile MVP **MUST** sustain background WS during active-cycle monitoring (Android foreground service; iOS background mode/entitlements). Desktop packaging via DEC-011 patterns.

## Technical Context

**Language/Version**: Dart 3.x / Flutter 3.22+ (FVM; desktop + mobile enabled)

**Primary Dependencies**:
- Flutter Material 3 + `flutter_localizations` (PT-BR / EN-US ARB)
- `packages/philarmony_core` — profiles, pin/hardware validators, WS DTOs/`KnownDevice`
- `web_socket_channel` — client to `ws://{host}/ws` (default port **80**)
- `multicast_dns` — `_philarmony._tcp`; manual host:port fallback (SSDP deferred)
- `flutter_riverpod` + `go_router` — sessions, shell routes
- `fl_chart` — live + history charts
- `drift` (+ sqlite) — history, known devices, pending commands
- `pdf` / `printing` (or equivalent) — **MVP PDF** reports (summary + charts)
- `flutter_local_notifications` — local alerts (remote push deferred)
- Background WS: Android FGS `dataSync`; iOS background networking entitlement/mode (App Store–acceptable; store risk accepted per clarify)
- Adaptive breakpoints: compact &lt;600, medium 600–1023, expanded ≥1024
- Packaging desktop: DEC-011 family (MSIX / DMG / AppImage+deb+rpm)

**Storage**: Drift SQLite + SharedPreferences; no cloud v1

**Testing**: `flutter_test` flow/widget + core unit tests; mock WS; manual smoke → `005-manual-validation` only

**Target Platform**: Win/macOS/Linux + Android 8+ + iOS 15+; shared screens, adaptive chrome

**Project Type**: Cross-platform Flutter application

**Performance Goals**: Dashboard &lt;1.5s after connect; charts ~60fps; WS handle &lt;10ms; Start→send &lt;500ms; memory &lt;150MB desktop / &lt;100MB mobile

**Constraints** (Clarifications 2026-08-10 locked):
- WS protocol **normative** = `specs/001-filament-dryer-esp32/contracts/websocket-api.md` (no dual-protocol; no stale 8080/`config/sensors`)
- Registry entity = **`KnownDevice`** (installer keeps `DeviceProfile`)
- History export MVP = **CSV and PDF** (PDF includes summary + charts)
- Background WS MVP = **required** on Android **and** iOS while monitoring active cycle
- Cycle control MVP = **Stop only** (confirm dialog); Pause/Resume out of scope
- Validators before any config/control send; GPLv3; PT-BR/EN-US; no auth/TLS v1

**Scale/Scope**: Discover, dashboard, start/stop, config, profiles, history (+exports), multi-device; max 3 concurrent WS (default)

## Constitution Check

*GATE: Must pass before Phase 0. Re-check after Phase 1.*

- [x] **Orientação a Objetos / Segurança de Hardware**: Typed WS client, sessions, validators; invalid payloads abort.
- [x] **Desempenho**: App budgets above; no device spam (1Hz status; debounce config).
- [x] **Failsafe**: Pin/temp validation; stop confirm; reconnect + queue UX; OS background denial surfaced.
- [x] **Workspace**: Flutter/FVM + mobile/desktop in `.vscode/workspace.json`.
- [x] **Teste Automatizado**: Flow/widget on this feature; manuals only in `005`.
- [x] **Documentação Sincronizada**: README via `after_plan`.
- [x] **Memória Compartilhada**: DEC-012 amended for clarify session.
- [x] **Commit approval / Cavemen / Languages**: unchanged.

**Post-design (clarify amend)**: Gates OK. Added justified complexity: PDF gen + mandatory iOS/Android background WS.

## Project Structure

### Documentation (this feature)

```text
specs/003-filament-dryer-control-app/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── ws-client.md
│   ├── adaptive-layout.md
│   ├── package-structure.md
│   └── history-export.md          # CSV + PDF MVP contract
└── tasks.md
```

### Source Code (repository root)

```text
packages/philarmony_core/          # + KnownDevice, WS codecs, cycle validators
apps/filament-dryer-control/       # single Flutter app — all OS
  lib/platform/                    # mDNS, notifications, BackgroundSession (iOS+Android)
  lib/features/history/            # list, detail, cycle_export.dart (CSV + PDF)
  … (see prior structure: shell, device, discover, dashboard, cycle, config, profiles)
```

**Structure Decision**: Unchanged — one Flutter app + shared core. Clarify adds PDF exporter under history and hardens `BackgroundSession` as MVP acceptance path (not best-effort).

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|--------------------------------------|
| Drift SQLite | Local-first history + offline queue | In-memory loses history |
| Riverpod + go_router | Multi-device sessions | setState insufficient |
| `lib/platform/` + FGS/iOS BG | Clarify: sustained bg WS required | Foreground-only fails FR-007a |
| PDF (+ charts) export | Clarify: CSV+PDF MVP | CSV-only rejected by product |
| Desktop installers + mobile | Compatibility matrix | Dev-only `flutter run` insufficient |

**Constitution compliance:** Philarmony; bilingual; GPLv3; manuals in 005.
