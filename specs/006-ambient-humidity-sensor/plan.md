# Implementation Plan: Ambient Humidity Sensor Support

**Branch**: `cursor/ambient-humidity-sensor-41a0` (feature alias `006-ambient-humidity-sensor`) | **Date**: 2026-08-19 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/006-ambient-humidity-sensor/spec.md`

## Summary

Add optional ambient temperature/humidity sensing to the Philarmony ESP32 firmware using the same sensor catalog and config style as chamber sensors (models may differ). When ambient humidity is valid, humidity-reduction exhaust stops once chamber RH is within a useless-ventilation margin of ambient (default 2 %RH), without changing chamber-only behavior, cycle completion rules, or mandatory cooldown. Soft-fail ambient faults to chamber-only fan logic. Expose ambient readings on `status/update`.

## Technical Context

**Language/Version**: C++17 / ESP-IDF v5.1+ (Arduino Core component) — same as 001

**Primary Dependencies**: Existing firmware stack (FreeRTOS, ESPAsyncWebServer, ArduinoJson, Preferences/NVS, DriverRegistry sensor factories)

**Storage**: NVS via ConfigManager — additive ambient fields on `SensorConfig`; optional `control.parameters.useless_ventilation_margin_rh`

**Testing**: PlatformIO Unity native/flow tests (`make test` / `pio test -e native`); automated tests owned by this product spec; manual smoke → `005-manual-validation`

**Target Platform**: ESP32 / ESP32-S3 (same board matrix as 001)

**Project Type**: Embedded firmware extension (additive to existing `src/` tree)

**Performance Goals**: Sensor sample path remains non-blocking; ambient read in same control loop budget as chamber (<20ms sample path); status still 1Hz; humidity-reduction stop within 5s of stable ambient-limited condition (SC-001)

**Constraints**: No dynamic allocation in control hot path; ambient failure MUST NOT force FAULT_STOPPED; cooldown MUST NOT be cancelled by ambient; max RAM discipline from constitution

**Scale/Scope**: One chamber + optional one ambient zone; firmware + contracts; forward-compatible status for 003/002 clients; ambient config UI polish in apps is follow-up, not blocking

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Orientação a Objetos e Segurança de Hardware**: Ambient uses existing `ISensorDriver` + DriverRegistry; validation via SafetyEngine-class checks; ambient soft-fail only
- [x] **Desempenho Máximo e Eficiência**: Second optional read in existing loop; no new blocking delays; static/config-time allocation of driver pointer
- [x] **Failsafe e Proteção de Hardware**: Chamber safety unchanged; ambient never sole cause of heater cut; cooldown preserved
- [x] **Workspace de Dependências e Configurações**: No new host toolchain; reuse PlatformIO/Make
- [x] **Teste Automatizado e Qualidade**: Flow tests on this product spec; open manuals → 005 only
- [x] **Documentação Sincronizada (README Vivo)**: after_plan hook syncs README
- [x] **Memória Compartilhada de Pesquisa e Conhecimento**: DEC-013 recorded
- [x] **Revisão e Aprovação Explícita de Commit**: Commit only after explicit authorization in interactive Speckit flows; cloud agent push follows environment policy for this run
- [x] **Cavemen Protocol**: Plan artifacts stay dense; no filler in agent completion noise beyond required report

**Post-design re-check**: Gates still pass. Complexity Tracking empty (no constitution violations requiring justification).

## Project Structure

### Documentation (this feature)

```text
specs/006-ambient-humidity-sensor/
├── plan.md              # This file
├── research.md          # Phase 0
├── data-model.md        # Phase 1
├── quickstart.md        # Phase 1
├── contracts/
│   ├── config-ambient.md
│   ├── status-ambient.md
│   └── humidity-exhaust-control.md
└── tasks.md             # Phase 2 (/speckit-tasks — NOT created here)
```

### Source Code (repository root — touch points)

```text
src/
├── core/
│   ├── ConfigManager.hpp/.cpp          # ambient_* fields + NVS
│   ├── HardwareConfigParser.hpp/.cpp  # placement + margin parse
│   ├── SafetyEngine.hpp/.cpp           # ambient validation soft-fail path
│   └── StateMachine.hpp/.cpp           # optional session telemetry fields
├── network/
│   ├── WebSocketServer.cpp             # status/update ambient keys
│   └── WebServer.cpp                   # GET hardware config round-trip ambient
├── main.cpp                            # ambient driver lifecycle + exhaust intents
└── drivers/sensors/                    # reuse existing catalog (no new exotic driver)

test/
└── …                                   # native flow tests: VS-A1–A4 style

.specify/memory/catalog/entries/
└── DEC-013-ambient-humidity-sensor.md
```

**Structure Decision**: Extend the existing single firmware tree from 001 (no new app package). Contracts live under `specs/006-…/contracts/` and normatively extend 001 websocket/config contracts.

## Complexity Tracking

> No constitution violations requiring justification.

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| — | — | — |

## Phase 0 / Phase 1 outputs

| Artifact | Path | Status |
|----------|------|--------|
| Research | [research.md](./research.md) | Complete |
| Data model | [data-model.md](./data-model.md) | Complete |
| Contracts | [contracts/](./contracts/) | Complete |
| Quickstart | [quickstart.md](./quickstart.md) | Complete |
| Memory | DEC-013 | Complete |

**Next command**: `/speckit-tasks` to generate `tasks.md` (implementation + automated tests; manuals → 005).

**Constitution compliance:** PROJECT_NAME = Philarmony, all constitutional provisions implemented as per `.specify/memory/constitution.md`
