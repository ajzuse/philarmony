# Quickstart Validation Ledger — Filament Dryer Control App

**Feature**: `003-filament-dryer-control-app`  
**Catalog**: Manual stages owned by `specs/005-manual-validation` (VS-CTRL-1…VS-CTRL-9)  
**Purpose**: Record PASS/FAIL/notes for human smoke. Do not track open work here — open tasks live only under 005.

| Stage | Title | Status | Date | Host / Device | Notes |
|-------|-------|--------|------|---------------|-------|
| VS-CTRL-1 | Discovery & connect | | | | Scan or manual → Connected → first status &lt;2s |
| VS-CTRL-2 | Start / stop (no pause) | | | | Start → drying → Stop with confirm; no Pause/Resume |
| VS-CTRL-3 | Config hardware | | | | Pin conflict blocked; ack/error topics per 001 |
| VS-CTRL-4 | History offline | | | | Completed cycle → offline → list + detail |
| VS-CTRL-5 | Adaptive layout | | | | Resize across 600 / 1024 per adaptive-layout contract |
| VS-CTRL-6 | Multi-device | | | | Two KnownDevices; switcher; max concurrent |
| VS-CTRL-7 | History export CSV + PDF | | | | Same cycle → both formats openable; PDF summary + charts |
| VS-CTRL-8 | Mobile background WS | | | | Active cycle ≥2 min backgrounded (iOS + Android) |
| VS-CTRL-9 | Packaging smoke (desktop) | | | | `make package-control-*` → launch artifact |

**Status values**: `PASS` · `FAIL` · `BLOCKED` · `SKIP` (with note)

**Preconditions (typical)**: Firmware `001` on LAN; `make run-control` or packaged build; for VS-CTRL-8 physical phone + OS permission prompts.
