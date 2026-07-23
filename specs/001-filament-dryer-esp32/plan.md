# Implementation Plan: Filament Dryer ESP32 Base Structure

**Branch**: `feature/001-filament-dryer-esp32` | **Date**: 2026-07-23 | **Spec**: `specs/001-filament-dryer-esp32/spec.md`

**Input**: Feature specification from `/specs/001-filament-dryer-esp32/spec.md` and User Requirements.

## Summary

Implement a high-performance, modular, and fault-tolerant firmware for an open-source DIY filament dryer running on ESP32 microcontrollers. The system uses a Klipper-inspired object-oriented architecture written in C++ (ESP-IDF / FreeRTOS) supporting extensible sensor, actuator, display, and plugin modules. It includes automatic WiFi fallback hotspot ("philarmony"), real-time WebSocket API, fixed HTTP log download paths (`/log/system`, `/log/drying`), and a fail-safe execution engine that halts heating while keeping system diagnostics fully operational upon any fault.

## Technical Context

**Language/Version**: C++17 / ESP-IDF v5.1+ (with Arduino Core component for library compatibility)

**Primary Dependencies**:
- FreeRTOS (Core ESP-IDF multitasking & synchronization)
- ESPAsyncWebServer & AsyncTCP (Non-blocking HTTP & WebSocket engine)
- ArduinoJson v6/v7 (High-performance JSON serialization/deserialization)
- LittleFS (Flash file system for log files and profile storage)
- Preferences / NVS (Non-volatile storage for pin/hardware configs)
- LovyanGFX / Adafruit GFX (Extensible display drivers for ST7789, ILI9341, SSD1306)

**Storage**:
- NVS (Non-Volatile Storage): Hardware pinout, WiFi credentials, filament profiles.
- LittleFS: System log (`system.log`) and Drying session log (`drying.log`).

**Testing**: Unity Test Framework (ESP-IDF / PlatformIO) for flow tests & simulated hardware injection.

**Target Platform**: ESP32, ESP32-S3, ESP32-D0WD (Tested on ESP32_DEVKITC_V4, LilyGo T-Display V1.1, ESP32-2432S028 CYD).

**Project Type**: Embedded Firmware (IoT / Hardware Controller).

**Performance Goals**:
- Telemetry & WebSocket broadcast: 1Hz ±20ms
- Sensor sampling loop: <20ms
- WebSocket command response: <50ms
- Maximum RAM Usage: <3.5MB Heap (<200KB Internal RAM, PSRAM optional)
- CPU Usage: <30% core average at 240MHz

**Constraints**:
- Absolute safety: Soft limit at 80°C, immediate MOSFET shutdown on fault/sensor error (<50ms response).
- Non-blocking architecture: Zero blocking delays in main loops.
- Dual log subsystem: HTTP endpoints `/log/system` and `/log/drying`.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Orientação a Objetos e Segurança de Hardware**: Hardware encapsulated in C++ driver objects (`SensorDriver`, `ActuatorDriver`, `DisplayDriver`) with strict parameter validation and safe abort triggers.
- [x] **Desempenho Máximo e Eficiência**: Deterministic algorithms, static allocation buffers for critical loops, FreeRTOS tasks with strict priorities.
- [x] **Failsafe e Proteção de Hardware**: Watchdog timers enabled, `FAULT_STOPPED` state immediately disables MOSFET outputs while preserving network/log diagnostics.
- [x] **Workspace de Dependências e Configurações**: Configuration maintained via PlatformIO environment (`platformio.ini`).
- [x] **Teste Automatizado e Qualidade**: End-to-end user flow scenario tests defined.
- [x] **Documentação Sincronizada (README Vivo)**: README updated automatically via hooks.
- [x] **Memória Compartilhada de Pesquisa e Conhecimento**: Technical decisions and hardware architecture documented in `research.md`.

## Project Structure

### Documentation (this feature)

```text
specs/001-filament-dryer-esp32/
├── spec.md              # Feature specification
├── plan.md              # Implementation plan (this file)
├── research.md          # Architecture decisions, benchmarks & hardware mapping
├── data-model.md        # Data structures, state machine & log entity schemas
├── quickstart.md        # Validation scenarios & test commands
└── contracts/
    ├── websocket-api.md # WebSocket API protocol & topics
    ├── http-api.md      # HTTP endpoints (/log/system, /log/drying, config)
    └── config-schema.json # Klipper-style hardware configuration schema
```

### Source Code (repository root)

```text
src/
├── main.cpp                     # Firmware entry point & system init
├── core/
│   ├── ConfigManager.hpp/.cpp   # NVS & JSON config manager (Klipper style)
│   ├── StateMachine.hpp/.cpp    # System & Drying cycle state machine
│   ├── SafetyEngine.hpp/.cpp    # Thermal runaway & fault supervisor
│   └── LogManager.hpp/.cpp      # Dual log recorder (system.log & drying.log)
├── drivers/
│   ├── interfaces/              # Driver abstract base classes
│   │   ├── ISensorDriver.hpp
│   │   ├── IActuatorDriver.hpp
│   │   └── IDisplayDriver.hpp
│   ├── sensors/                 # Sensor implementations
│   │   ├── SHT31Sensor.hpp/.cpp
│   │   ├── DHT22Sensor.hpp/.cpp
│   │   ├── DS18B20Sensor.hpp/.cpp
│   │   └── AnalogThermistor.hpp/.cpp
│   ├── actuators/               # MOSFET & PWM actuators
│   │   ├── MosfetActuator.hpp/.cpp
│   │   └── FanActuator.hpp/.cpp
│   └── display/                 # Display implementations
│       ├── SSD1306Display.hpp/.cpp
│       ├── ST7789Display.hpp/.cpp
│       └── ILI9341Display.hpp/.cpp
├── plugins/                     # Plugin extension architecture
│   ├── PluginManager.hpp/.cpp
│   └── IPlugin.hpp
├── network/
│   ├── WifiManager.hpp/.cpp     # STA connection & AP "philarmony" fallback
│   ├── WebServer.hpp/.cpp       # Async HTTP server & log download handlers
│   └── WebSocketServer.hpp/.cpp # 1Hz status stream & API controller
└── utils/
    ├── SystemMetrics.hpp/.cpp   # CPU & Heap memory usage monitor
    └── TimeUtils.hpp/.cpp

include/
└── firmware_version.h

test/
├── test_state_machine/
├── test_safety_engine/
├── test_log_manager/
└── test_config_parser/
```

**Structure Decision**: Single embedded firmware project targeting ESP32 architectures using C++/PlatformIO.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| Dual log files (`system.log` and `drying.log`) | User requires separate operational log preserved post-mortem across reboots/failures | Single RAM log is lost on reset; single unified log gets cluttered with boot messages during active drying analysis |
| Klipper-inspired driver plugin architecture | User requires support for diverse/expandable sensors, displays, and actuators | Hardcoded driver calls prevent adding custom hardware without rewriting core loop |

**Constitution compliance:** PROJECT_NAME = Philarmony, all constitutional provisions implemented as per `.specify/memory/constitution.md`.
