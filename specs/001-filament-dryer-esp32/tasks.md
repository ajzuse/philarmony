# Tasks: Filament Dryer ESP32 Base Structure

**Input**: Design documents from `/specs/001-filament-dryer-esp32/`
**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`

---

## Format: `- [ ] [ID] [P?] [Story?] Description with file path`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story identifier ([US1], [US2], [US3], [US4], [US5])

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization, PlatformIO build environment, partition table, and version headers.

- [ ] T001 Create PlatformIO project structure and platformio.ini for target ESP32 environments in platformio.ini
- [ ] T002 [P] Create firmware version header in include/firmware_version.h
- [ ] T003 [P] Configure LittleFS partition table layout in partitions.csv

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented.

- [ ] T004 [P] Implement ConfigManager for NVS storage and Klipper-style JSON config in src/core/ConfigManager.hpp and src/core/ConfigManager.cpp
- [ ] T005 [P] Implement StateMachine for system and drying session states in src/core/StateMachine.hpp and src/core/StateMachine.cpp
- [ ] T006 [P] Implement dual LogManager for system.log and drying.log in LittleFS in src/core/LogManager.hpp and src/core/LogManager.cpp
- [ ] T007 Implement SafetyEngine with hard 80C limit cutoff and hardware watchdog in src/core/SafetyEngine.hpp and src/core/SafetyEngine.cpp
- [ ] T008 [P] Implement abstract driver interfaces ISensorDriver, IActuatorDriver, and IDisplayDriver in src/drivers/interfaces/
- [ ] T009 [P] Implement PluginManager and IPlugin hook extension interface in src/plugins/PluginManager.hpp and src/plugins/IPlugin.hpp
- [ ] T010 [P] Implement SystemMetrics for FreeRTOS CPU and Heap utilization in src/utils/SystemMetrics.hpp and src/utils/SystemMetrics.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel.

---

## Phase 3: User Story 1 - Initial Setup via Hotspot (Priority: P1) 🎯 MVP

**Goal**: WiFi connection with automatic fallback to AP hotspot "philarmony" (192.168.4.1) serving web configuration page.

**Independent Test**: Boot unconfigured device -> Connect to "philarmony" AP -> Submit WiFi credentials -> ESP32 connects to STA network and disables AP.

- [ ] T011 [P] [US1] Implement WifiManager with STA connection logic and AP fallback in src/network/WifiManager.hpp and src/network/WifiManager.cpp
- [ ] T012 [US1] Implement Async HTTP WebServer serving captive portal configuration page in src/network/WebServer.hpp and src/network/WebServer.cpp
- [ ] T013 [US1] Implement POST /api/wifi/config credential handling and NVS save in src/network/WebServer.cpp

**Checkpoint**: User Story 1 (WiFi Setup Hotspot) complete and testable independently.

---

## Phase 4: User Story 2 - Normal Operation & WebSocket Real-Time Control (Priority: P1)

**Goal**: WebSocket API for hardware/sensor configuration, drying cycle control, and 1Hz status updates.

**Independent Test**: Connect via WebSocket -> Send hardware config for SHT31 and MOSFET AOD4184 -> Issue control/start -> Verify PWM driving and 1Hz status stream.

- [ ] T014 [P] [US2] Implement SHT31 I2C sensor driver in src/drivers/sensors/SHT31Sensor.hpp and src/drivers/sensors/SHT31Sensor.cpp
- [ ] T015 [P] [US2] Implement DHT22 and DS18B20 drivers in src/drivers/sensors/DHT22Sensor.cpp and src/drivers/sensors/DS18B20Sensor.cpp
- [ ] T016 [P] [US2] Implement MOSFET AOD4184 PWM heater driver in src/drivers/actuators/MosfetActuator.hpp and src/drivers/actuators/MosfetActuator.cpp
- [ ] T017 [P] [US2] Implement Fan driver (PWM / digital / shared MOSFET) in src/drivers/actuators/FanActuator.hpp and src/drivers/actuators/FanActuator.cpp
- [ ] T018 [US2] Implement WebSocketServer handling control/start, control/stop, and config/hardware in src/network/WebSocketServer.hpp and src/network/WebSocketServer.cpp
- [ ] T019 [US2] Implement 1Hz status update broadcast on topic status/update in src/network/WebSocketServer.cpp

**Checkpoint**: User Story 2 complete and testable independently.

---

## Phase 5: User Story 3 - Fault Tolerance, Safety Engine & Fixed HTTP Logs (Priority: P1)

**Goal**: Immediate safe abort on fault (<50ms MOSFET cutoff) while maintaining platform/network, plus fixed HTTP log downloads (/log/system and /log/drying).

**Independent Test**: Disconnect sensor during cycle -> Heater PWM drops to 0% -> System state moves to FAULT_STOPPED -> Download log via GET /log/drying and verify fault reason.

- [ ] T020 [US3] Implement thermal runaway and sensor timeout detection logic in src/core/SafetyEngine.cpp
- [ ] T021 [US3] Implement safe abort emergency handler forcing PWM outputs LOW in src/core/SafetyEngine.cpp
- [ ] T022 [P] [US3] Implement fixed HTTP endpoint GET /log/drying in src/network/WebServer.cpp
- [ ] T023 [P] [US3] Implement fixed HTTP endpoint GET /log/system in src/network/WebServer.cpp
- [ ] T024 [US3] Implement WebSocket real-time log streaming on topic logs/stream in src/network/WebSocketServer.cpp

**Checkpoint**: User Story 3 complete and testable independently.

---

## Phase 6: User Story 4 - Multi-Display Support & Layout Engine (Priority: P2)

**Goal**: Display telemetry and status on attached ST7789, ILI9341, or SSD1306 displays via LovyanGFX zero-copy DMA.

**Independent Test**: Send config/display via WebSocket -> Display renders status fields at 1Hz without blocking control loop.

- [ ] T025 [P] [US4] Implement ST7789 TFT display driver in src/drivers/display/ST7789Display.hpp and src/drivers/display/ST7789Display.cpp
- [ ] T026 [P] [US4] Implement ILI9341 display driver for CYD board in src/drivers/display/ILI9341Display.cpp
- [ ] T027 [P] [US4] Implement SSD1306 OLED display driver in src/drivers/display/SSD1306Display.cpp
- [ ] T028 [US4] Implement auto-adapting display layout manager in src/drivers/display/DisplayManager.cpp

**Checkpoint**: User Story 4 complete and testable independently.

---

## Phase 7: User Story 5 - Filament Drying Profiles Management (Priority: P2)

**Goal**: Built-in material presets (PLA, PETG, ABS, TPU, Nylon) and custom profile management via WebSocket API.

**Independent Test**: Create custom profile via WebSocket -> Start session using profile ID -> Parameters applied to drying run.

- [ ] T029 [P] [US5] Implement built-in material profiles in src/core/ProfileManager.hpp and src/core/ProfileManager.cpp
- [ ] T030 [US5] Implement custom profile NVS persistence and WebSocket API handlers in src/network/WebSocketServer.cpp

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Documentation, memory optimization, and quickstart validation.

- [ ] T031 [P] Update firmware documentation and API references in README.md
- [ ] T032 Perform memory leak profiling and FreeRTOS stack verification
- [ ] T033 Execute end-to-end quickstart validation scenarios in specs/001-filament-dryer-esp32/quickstart.md

---

## Dependencies & Execution Order

1. **Phase 1 (Setup)** -> Blocks Phase 2
2. **Phase 2 (Foundational)** -> Blocks all User Stories
3. **Phase 3 (US1)** -> Priority P1 (MVP)
4. **Phase 4 (US2)** -> Priority P1
5. **Phase 5 (US3)** -> Priority P1
6. **Phase 6 (US4)** -> Priority P2
7. **Phase 7 (US5)** -> Priority P2
8. **Phase 8 (Polish)** -> Final

---

## Implementation Strategy

### MVP First (Phases 1-3)
Complete Setup, Foundational, and User Story 1 (WiFi Setup Hotspot) to produce a bootable, configurable hardware node.

### Incremental Delivery
Add US2 (Control & Telemetry) -> Add US3 (Safety Engine & Logs) -> Add US4 (Displays) -> Add US5 (Profiles).
