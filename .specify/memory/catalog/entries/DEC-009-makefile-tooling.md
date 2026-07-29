# DEC-009 — Root Makefile over PlatformIO

**Date**: 2026-07-28  
**Feature**: `001-filament-dryer-esp32`  
**Status**: Accepted (plan)

## Decision
Provide a repository-root GNU `Makefile` that wraps PlatformIO (`pio`) for compile, native Unity tests, and ESP32 flash/install. `platformio.ini` remains the toolchain source of truth.

## Rationale
- Shorter contributor commands (`make build|test|flash`).
- Propagates `pio` exit codes for CI / constitution test gates.
- Board selection via `ENV=` without duplicating env definitions.

## Alternatives rejected
Shell-only scripts; CMake primary; Task/just; docs-only `pio` usage.

## Artifacts
- `specs/001-filament-dryer-esp32/plan.md` (Makefile target map)
- `specs/001-filament-dryer-esp32/research.md` §8
- `specs/001-filament-dryer-esp32/contracts/makefile-targets.md`
- Implementation: root `Makefile` + workspace/README/quickstart updates (via `/speckit-tasks` → implement)
