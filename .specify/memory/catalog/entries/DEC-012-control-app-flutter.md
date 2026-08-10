# DEC-012 — Control App Flutter Stack (Adaptive Multi-Platform)

**Date**: 2026-08-10  
**Amended**: 2026-08-10 — Clarifications Session  
**Feature**: `003-filament-dryer-control-app`  
**Status**: Accepted (amended)

## Decision

Ship the Philarmony dryer **control app** as a **single Flutter application** (`apps/filament-dryer-control`) targeting Windows, macOS, Linux, Android, and iOS, with **adaptive layouts** and shared UI/domain via `packages/philarmony_core`.

Locked by Clarifications 2026-08-10:
- WebSocket client follows firmware **`001` only** (`websocket-api.md`)
- Registry entity = **`KnownDevice`** (installer keeps `DeviceProfile`)
- History export MVP = **CSV + PDF** (summary + charts)
- Background WS MVP = **required** on Android **and** iOS during active-cycle monitoring
- Cycle control MVP = **Stop only** (Pause/Resume out of scope)

Stack: Drift, Riverpod, go_router, fl_chart, pdf/printing, local notifications.

## Rationale

User shared-codebase requirement + DEC-010 continuity + clarify session removes prior “best-effort” / CSV-only ambiguities.

## Alternatives rejected

| Option | Why rejected |
|--------|----------------|
| Separate mobile & desktop apps | Duplication |
| Dual WS protocol / port 8080 legacy | Clarify → 001 only |
| CSV-only history export | Clarify → CSV+PDF |
| iOS foreground-only bg WS | Clarify → required both platforms |
| Pause/Resume in MVP | Clarify → Stop only |

## Consequences

- Tasks/plan must include PDF exporter + hardened `BackgroundSession`
- Store entitlements/docs mandatory for iOS/Android background
- Manual stages for bg WS + PDF → `005-manual-validation`
- Catalog + `specs/003-*/research.md` are source of truth
