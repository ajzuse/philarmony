# Contract: Package Structure (Monorepo)

**Feature**: `003-filament-dryer-control-app` | **Date**: 2026-08-10  
**Related**: DEC-010, DEC-012

## Packages

| Path | Role | Depends on |
|------|------|------------|
| `packages/philarmony_core` | Domain models, validators, WS DTOs/codecs (pure Dart) | none (Flutter-free) |
| `apps/esp32-desktop-installer` | Feature 002 installer | `philarmony_core` |
| `apps/filament-dryer-control` | Feature 003 control app (all OS) | `philarmony_core`, Flutter plugins |

## Control app module boundaries

```text
lib/
  device/     → implements discovery + PhilarmonyWsClient (uses core codecs)
  data/       → Drift DB / repositories
  features/*  → UI + Riverpod notifiers (no direct raw sockets)
  shell/      → AdaptiveScaffold only
  platform/   → MdnsDiscovery, LocalNotifications, BackgroundSession (interfaces)
```

## Dependency rules

1. `features/*` MUST NOT import `dart:io` Platform for layout.  
2. `philarmony_core` MUST NOT depend on Flutter.  
3. WS JSON field names MUST match 001 contracts (codec tests).  
4. Installer and control app MAY share validators; control app MUST NOT import installer UI.

## Pub workspace (recommended)

Root or app `pubspec.yaml` path dependency:

```yaml
dependencies:
  philarmony_core:
    path: ../../packages/philarmony_core
```

Optional Melos later; not required for MVP.
