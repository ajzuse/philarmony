# DEC-001: Language & Runtime

**Tags:** architecture, language, freertos  
**Source:** `specs/001-filament-dryer-esp32/research.md` §1  
**Updated:** 2026-07-28

## Decision
C++17 with ESP-IDF v5.1+ (Arduino as component). FreeRTOS dual-core: Core 0 network I/O, Core 1 control/safety/display.

## Alternatives rejected
- `esp-rs`: immature display/AsyncWebServer ecosystem for this matrix.
- MicroPython: GC pauses breach 20ms control-loop budget.

## Implications
No dynamic allocation in the critical control path; prefer static buffers.
