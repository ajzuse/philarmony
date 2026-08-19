# Shared Memory Catalog

Versioned knowledge base for Philarmony agents and developers.
Source seed: `specs/001-filament-dryer-esp32/research.md` (T100).

## Index

| ID | Title | Tags | Updated |
|----|-------|------|---------|
| DEC-001 | C++17 + ESP-IDF/Arduino FreeRTOS dual-core | architecture, language, freertos | 2026-07-28 |
| DEC-002 | Plugin-driver ObjectRegistry architecture | architecture, drivers, plugins | 2026-07-28 |
| DEC-003 | Hardware support matrix (sensors/actuators/displays) | hardware, drivers | 2026-07-28 |
| DEC-004 | Fault-tolerance & SafetyEngine matrix | safety, failsafe | 2026-07-28 |
| DEC-005 | Dual logging (system.log / drying.log) | logging, littlefs | 2026-07-28 |
| DEC-006 | Ziegler-Nichols PID auto-calibrate | control, pid | 2026-07-28 |
| DEC-007 | Single-app partitions (OTA out of scope) | flash, partitions | 2026-07-28 |
| DEC-008 | WiFi ConfigManager sole NVS owner + STA fail-fast | wifi, nvs, sc-01 | 2026-07-28 |
| DEC-009 | Root Makefile wraps PlatformIO (build/test/flash) | tooling, makefile, pio | 2026-07-28 |
| DEC-010 | Desktop installer Flutter + esptool (shared w/ 003) | desktop, installer, flutter | 2026-07-28 |
| DEC-011 | Host installers MSIX/DMG + Linux Make/AppImage/deb/rpm | packaging, windows, macos, linux, rpm | 2026-07-28 |
| DEC-012 | Control app Flutter adaptive multi-platform (CSV+PDF, bg WS iOS+Android, Stop-only) | flutter, control-app, adaptive, websocket | 2026-08-10 |
| DEC-013 | On-device touch UI (LVGL+LovyanGFX) + PAUSED state / pause-resume API | touchscreen, lvgl, pause, ui, esp32 | 2026-08-19 |

See individual entry files under `entries/`.
