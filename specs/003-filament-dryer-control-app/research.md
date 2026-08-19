# Research: Filament Dryer Control App

**Feature**: `003-filament-dryer-control-app` | **Date**: 2026-08-10  
**Amended**: 2026-08-10 — Clarifications Session locked into research (R2/R7/R11/R12)  
**User direction**: Flutter app for mobile + desktop, maximize shared code across phone, tablet, and desktop.

## R1 — Single Flutter codebase (desktop + mobile + tablet)

**Decision**: One Flutter application `apps/filament-dryer-control` with all platform runners enabled (android, ios, windows, macos, linux). Shared feature UI; adaptive chrome by breakpoint; platform adapters only under `lib/platform/`.

**Rationale**: User priority + DEC-010/012.

**Breakpoint policy** (see `contracts/adaptive-layout.md`): compact &lt;600 → bottom nav; medium 600–1023 → rail; expanded ≥1024 → rail + master–detail.

**Alternatives considered**: Separate apps; RN/MAUI/Tauri hybrid; web-only — rejected.

## R2 — Canonical WebSocket protocol (firmware 001)

**Decision**: Client MUST implement `specs/001-filament-dryer-esp32/contracts/websocket-api.md` as **sole** normative API. Spec Clarifications 2026-08-10: no dual-protocol; appendix corrected to port **80**, `/ws`, `config/hardware`, `max_duration_min`.

| Concern | Client MUST |
|---------|-------------|
| URL | `ws://{host}/ws` default port 80; manual host:port override allowed |
| Hardware | `config/hardware` (+ `config/display` / `config/control` as in 001) |
| Start | `max_duration_min` (not `max_time_min`) |
| Profiles | `config/profiles/*` per 001 |

**Alternatives considered**: Dual-protocol adapter — **rejected** (clarify Option A).

## R3 — Discovery (mDNS + manual)

**Decision**: mDNS `_philarmony._tcp` + manual IP/hostname. SSDP deferred.

## R4 — State management & navigation

**Decision**: `flutter_riverpod` + `go_router` StatefulShellRoute.

## R5 — Local persistence

**Decision**: Drift for `KnownDevice`, `DryingCycle`, `CycleSample`, `PendingCommand`.

## R6 — Charts & telemetry buffer

**Decision**: `fl_chart`; 60‑min in-memory ring @ 1Hz; downsample on cycle end.

## R7 — Notifications & background WebSocket

**Decision** (clarify 2026-08-10):
- **Local** notifications MVP (`flutter_local_notifications`); remote push deferred.
- **Background WS is MVP-required** on **Android and iOS** while an active cycle is monitored (or user opted to keep monitoring):
  - Android: foreground service type `dataSync` (or documented equivalent)
  - iOS: declared background mode / entitlements for sustained LAN WS (VoIP or other App Store–acceptable mode); follow platform guidelines; store-review risk **accepted**
- If OS revokes background execution → clear error UX + reconnect on foreground

**Alternatives considered**: iOS foreground-only / no background — **rejected** (clarify Option A).

## R8 — Sharing with installer (`philarmony_core`)

**Decision**: Installer `DeviceProfile` unchanged; control registry = **`KnownDevice`** (clarify Option A). Extend core with WS DTOs + cycle validators.

## R9 — Packaging & CI

**Decision**: Desktop DEC-011 patterns; mobile CI artifacts; `flutter test`/`analyze` on PRs.

## R10 — Testing placement (constitution)

**Decision**: Automated tests on feature 003; open manuals → `005-manual-validation`.

## R11 — History export (CSV + PDF)

**Decision** (clarify 2026-08-10): MVP **requires both** CSV (all fields) and PDF (summary + charts). Use `pdf`/`printing` (or equivalent) to embed chart bitmaps or vector series from persisted samples. See `contracts/history-export.md`.

**Alternatives considered**: CSV-only MVP — **rejected** (clarify Option A).

## R12 — Cycle control: Stop only

**Decision** (clarify 2026-08-10): MVP exposes **Stop** with confirmation only. No Pause/Resume UI until firmware documents a control topic. Out of Scope in spec.

**Alternatives considered**: Pause required / disabled stub — rejected / not chosen (Option B = Stop only).

## Resolved unknowns

| Topic | Resolution |
|-------|------------|
| Framework | Flutter |
| Code sharing | Single app + adaptive |
| WS contract | 001 only |
| Registry name | KnownDevice |
| Export MVP | CSV + PDF |
| Background WS | Required iOS+Android |
| Pause/Resume | Out of MVP |
| State/DB/charts | Riverpod / Drift / fl_chart |
| Push remote | Deferred |
