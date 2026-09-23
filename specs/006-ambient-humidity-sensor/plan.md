# Implementation Plan: Ambient Humidity Sensor Support

**Branch**: `006-ambient-humidity-sensor` | **Date**: 2026-09-03 | **Spec**: `specs/006-ambient-humidity-sensor/spec.md`

**Input**: Feature specification from `/specs/006-ambient-humidity-sensor/spec.md`

## Summary

Add optional ambient temperature/humidity sensor support to the ESP32 filament dryer. When present, the exhaust fan uses ambient humidity to decide when ventilation is useless (chamber ≤ ambient + configurable margin → fan stops). Reuses existing `ISensorDriver` catalog — no new driver code. Chamber-only builds remain fully supported. Ambient readings exposed on touch UI and WebSocket status/update.

## Technical Context

**Language/Version**: C++17 / ESP-IDF v5.1+ (Arduino Core via PlatformIO) — same as 001

**Primary Dependencies**:
- Existing: FreeRTOS, ArduinoJson, Preferences/NVS, ESPAsyncWebServer, LovyanGFX, LVGL
- Sensor drivers: SHT31, SHT3x, BME280, AHT20, DHT22, DS18B20, NTC, Custom (all via `Wire.h`)
- No new library dependencies required

**Storage**:
- NVS: `SensorConfig` struct (ambient fields added to existing `"sensor"` key)
- No new storage entities

**Testing**:
- Automated: Unity native + host flow tests for config parser ambient parsing, fan stop logic, NVS persistence
- Manual: `005-manual-validation` VS-UI-* for on-device sensor read

**Target Platform**: ESP32 / ESP32-S3 with chamber sensor + optional ambient sensor

**Project Type**: Embedded firmware sensor extension (same binary as 001)

**Performance Goals**:
- Ambient reading latency: same as chamber (1Hz control loop tick)
- Fan stop response: ≤5s from ambient condition stably met (SC-001)
- Fallback to chamber-only: ≤30s after ambient sensor loss (SC-004)
- Zero additional RAM overhead when ambient not configured

**Constraints**:
- SafetyEngine authority unchanged; ambient does not gate heater safety
- No new library dependencies
- Ambient-aware logic applies only to humidity-reduction exhaust, not cycle stop criteria
- Configurable margin: 0–10%RH, default 2%RH

**Scale/Scope**:
- Single ambient zone (one ambient sensor max)
- Same sensor catalog as chamber (8 driver types)
- Touch UI + WebSocket status exposure

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Orientação a Objetos e Segurança de Hardware**: Ambient sensor encapsulated in `ISensorDriver`; fan decision logic uses validated readings with timeout fallback.
- [x] **Desempenho Máximo e Eficiência**: No dynamic allocation in control path; ambient reading reuses existing 1Hz tick; zero overhead when unconfigured.
- [x] **Failsafe e Proteção de Hardware**: 30s timeout fallback to chamber-only; parameter validation on margin (0–10%); SafetyEngine unchanged.
- [x] **Workspace de Dependências**: No new deps; existing sensor libs already in `platformio.ini`.
- [x] **Teste Automatizado e Qualidade**: Flow tests for ambient fan stop, config parser, NVS round-trip; manual smoke → `005`.
- [x] **Documentação Sincronizada**: README via `after_plan` hook.
- [x] **Revisão e Aprovação Explícita de Commit**: Unchanged process rules.

**Post-design**: Gates OK. No complexity violations — ambient sensor is additive, not structural.

## Project Structure

### Documentation (this feature)

```text
specs/006-ambient-humidity-sensor/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/
│   └── ambient-sensor-api.md
├── checklists/
│   └── requirements.md
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
src/
├── core/
│   ├── ConfigManager.hpp/.cpp          # + ambient sensor fields in SensorConfig
│   ├── HardwareConfigParser.hpp/.cpp   # + parse "role": "ambient" sensor objects
│   └── DriverRegistry.hpp/.cpp         # Unchanged (already supports all types)
├── drivers/
│   ├── sensors/                        # Unchanged (all 8 drivers reusable)
│   └── interfaces/
│       └── IDriverInterfaces.hpp       # Unchanged (ISensorDriver + SensorReading)
├── main.cpp                            # + ambientSensor pointer, init, 1Hz read, fan logic, telemetry
├── network/
│   └── WebSocketServer.cpp             # + ambient_temp_c / ambient_humidity_pct in status/update
├── drivers/display/
│   └── DisplayManager.cpp              # + ambient fields in layout
└── ui/
    ├── UiApp.hpp/.cpp                  # + live_ambient_temp_c_ / live_ambient_humidity_pct_
    └── screens/
        ├── screen_home.cpp             # + ambient readings (if space)
        ├── screen_monitoring.cpp       # + ambient readings
        └── screen_settings_sensors.cpp # + ambient sensor config display

test/
├── test_ambient_config_parser/         # Ambient sensor JSON parsing tests
├── test_ambient_nvs_roundtrip/         # ConfigManager ambient NVS persist/load
└── test_ambient_fan_stop/              # Fan stop logic with ambient comparison
```

**Structure Decision**: Extend the single firmware tree from 001. Add ambient sensor fields to existing `SensorConfig`; insert ambient-aware fan logic into existing `controlLoopTask`. No new source files needed — all changes are additions to existing files.

## Complexity Tracking

No constitution violations. Ambient sensor feature is purely additive:
- Reuses existing `ISensorDriver` interface and `DriverRegistry` factory
- Adds fields to existing `SensorConfig` struct
- Inserts conditional fan-stop check into existing control loop
- No new dependencies, no new architectural patterns

**Constitution compliance:** PROJECT_NAME = Philarmony, provisions per `.specify/memory/constitution.md`
