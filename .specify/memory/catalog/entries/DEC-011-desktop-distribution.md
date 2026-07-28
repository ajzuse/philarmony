# DEC-011 — Host Desktop Distribution (MSIX / DMG / Linux Make + AppImage/deb/rpm)

**Date**: 2026-07-28  
**Amended**: 2026-07-28 — Linux first-class: Makefile + AppImage + deb + **rpm** (Fedora/RHEL-family)  
**Feature**: `002-esp32-desktop-installer`  
**Status**: Accepted  
**Related**: DEC-010 (Flutter stack)

## Decision

Distribute the Flutter **Philarmony Desktop Installer** as **single-file OS installers**:

- **Windows**: `.msix` (primary), optional Inno `.exe` fallback  
- **macOS**: signed + notarized `.dmg`  
- **Linux** (first-class):
  - **Build**: repo **`make package-installer-linux*`** targets
  - **Ship**: **AppImage** (portable), **`.deb`** (Debian/Ubuntu), **`.rpm`** (Fedora, RHEL, Rocky, Alma, and other rpm-based)

Published on GitHub Releases with checksums. App bundle embeds firmware + esptool.

## Rationale

End users must install without Flutter SDK or raw `Release/` folders. Win/macOS/Linux all need native install UX. DIY users on Fedora and other Red Hat–based distros expect `.rpm` + `dnf`/`yum`, not only AppImage. Makefile keeps packaging discoverable alongside firmware `make` targets (DEC-009).

## Alternatives rejected

| Option | Why rejected |
|--------|----------------|
| ZIP of Flutter build output | Poor UX; macOS Gatekeeper friction |
| Store-only distribution | Too slow for DIY/GPLv3 audience; sideload required |
| Web-based flash | Spec requires USB; browsers lack reliable serial flash |
| Linux AppImage-only | Misses `apt`/`dnf` users; Red Hat family underserved |
| Make as end-user install | Users should download packages; Make is for builders/CI |

## Consequences

- CI release workflow builds/packages Win + macOS + Linux (3 Linux artifacts)
- Root Makefile gains installer packaging targets (alongside firmware)
- Signing secrets required for public macOS (and preferred for Windows); optional GPG for Linux packages
- Docs must include Download & Install for Win/macOS/`dnf`/`apt`/AppImage (PT-BR/EN-US)
- Same packaging approach reusable for `003` desktop builds
