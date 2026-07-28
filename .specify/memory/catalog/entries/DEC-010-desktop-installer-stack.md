# DEC-010 — Desktop Installer Stack (Flutter, shared with Control App)

**Date**: 2026-07-28  
**Amended**: 2026-07-28  
**Feature**: `002-esp32-desktop-installer`  
**Status**: Accepted (amended)

## Decision

Use **Flutter (Dart)** for the Philarmony ESP32 desktop installer, with shared packages (`philarmony_core`) consumed by feature `003-filament-dryer-control-app`. USB enumeration via **flutter_libserialport**; flashing via **bundled esptool** driven from Dart `Process`. Pre-built firmware from feature `001`.

## Rationale

- Product direction: one UI/domain codebase from installer → desktop/mobile dryer control.
- Spec `003` already specifies Flutter for multi-platform control.
- Flutter covers wizard UX, i18n, desktop targets, and serial listing; Espressif flash remains esptool (proven), not reimplemented.

## Supersedes

Earlier draft of DEC-010 recommending Tauri 2 + React + espflash-in-process. That approach remains technically valid for a standalone flasher but conflicts with shared-codebase goals.

## Alternatives rejected

| Option | Why rejected |
|--------|----------------|
| Tauri 2 + React | Split stack vs `003` Flutter |
| Electron + esptool.py | Large runtime; no Flutter reuse |
| Pure-Dart ESP flash protocol | High risk / maintenance |

## Related

- **DEC-011** — Host OS installers: MSIX / DMG / Linux Make + AppImage + deb + **rpm** (Fedora/RHEL-family)

## Consequences

- Monorepo: `packages/philarmony_core`, `apps/esp32-desktop-installer`
- Workspace documents Flutter SDK + esptool asset packaging
- Installer MVP is **desktop-only** for USB flash; mobile control in `003` uses network/WS
- Host packaging (DEC-011) is required for end-user delivery, not optional polish
- Catalog + plan/research are source of truth for this DEC
