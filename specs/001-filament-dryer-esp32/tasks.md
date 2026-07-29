# Tasks: Filament Dryer ESP32 Base Structure

**Input**: Design documents from `/specs/001-filament-dryer-esp32/`
**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`

---

## Format: `- [ ] [ID] [P?] [Story?] Description with file path`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story identifier ([US1], [US2], [US3], [US4], [US5], [US6], [US7], [US8])

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization, PlatformIO build environment, partition table, and version headers.

- [x] T001 Create PlatformIO project structure and platformio.ini for target ESP32 environments in platformio.ini
- [x] T002 [P] Create firmware version header in include/firmware_version.h
- [x] T003 [P] Configure LittleFS partition table layout in partitions.csv

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented.

- [x] T004 [P] Implement ConfigManager for NVS storage and Klipper-style JSON config in src/core/ConfigManager.hpp and src/core/ConfigManager.cpp
- [x] T005 [P] Implement StateMachine for system and drying session states in src/core/StateMachine.hpp and src/core/StateMachine.cpp
- [x] T006 [P] Implement dual LogManager for system.log and drying.log in LittleFS in src/core/LogManager.hpp and src/core/LogManager.cpp
- [x] T007 Implement SafetyEngine with hard 80C limit cutoff and hardware watchdog in src/core/SafetyEngine.hpp and src/core/SafetyEngine.cpp
- [x] T008 [P] Implement abstract driver interfaces ISensorDriver, IActuatorDriver, and IDisplayDriver in src/drivers/interfaces/
- [x] T009 [P] Implement PluginManager and IPlugin hook extension interface in src/plugins/PluginManager.hpp and src/plugins/IPlugin.hpp
- [x] T010 [P] Implement SystemMetrics for FreeRTOS CPU and Heap utilization in src/utils/SystemMetrics.hpp and src/utils/SystemMetrics.cpp

**NEW: Generic Driver Registry & Factory Pattern**
- [x] T010a [P] Implement DriverRegistry with factory functions for sensors, actuators, displays, and control algorithms in src/core/DriverRegistry.hpp and src/core/DriverRegistry.cpp
- [x] T010b [P] Implement built-in driver registration (sensors, actuators, displays, control algorithms) in src/core/DriverRegistry.cpp
- [x] T010c [P] Implement generic hardware configuration parser and validator in src/core/HardwareConfigParser.hpp and src/core/HardwareConfigParser.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel.

---

## Phase 3: User Story 1 - Initial Setup via Hotspot (Priority: P1) 🎯 MVP

**Goal**: WiFi connection with automatic fallback to AP hotspot "philarmony" (192.168.4.1) serving web configuration page.

**Independent Test**: Boot unconfigured device -> Connect to "philarmony" AP -> Submit WiFi credentials -> ESP32 connects to STA network and disables AP.

- [x] T011 [P] [US1] Implement WifiManager with STA connection logic and AP fallback in src/network/WifiManager.hpp and src/network/WifiManager.cpp
- [x] T012 [US1] Implement Async HTTP WebServer serving captive portal configuration page in src/network/WebServer.hpp and src/network/WebServer.cpp
- [x] T013 [US1] Implement POST /api/wifi/config credential handling and NVS save in src/network/WebServer.cpp

**Checkpoint**: User Story 1 (WiFi Setup Hotspot) complete and testable independently.

---

## Phase 4: User Story 2 - Generic Hardware Configuration & WebSocket Real-Time Control (Priority: P1)

**Goal**: Generic WebSocket API for hardware/sensor/actuator/display/control configuration, drying cycle control, and 1Hz status updates.

**Independent Test**: Connect via WebSocket -> Send generic hardware config for any sensor/actuator/display -> Issue control/start -> Verify PWM driving and 1Hz status stream.

- [x] T014 [P] [US2] Implement SHT3x I2C sensor driver in src/drivers/sensors/SHT3xSensor.hpp and src/drivers/sensors/SHT3xSensor.cpp
- [x] T015 [P] [US2] Implement DHT22 and DS18B20 drivers in src/drivers/sensors/DHT22Sensor.cpp and src/drivers/sensors/DS18B20Sensor.cpp
- [x] T015a [P] [US2] Implement NTC thermistor ADC sensor driver in src/drivers/sensors/NTCSensor.hpp and src/drivers/sensors/NTCSensor.cpp
- [x] T015b [P] [US2] Implement BME280/BMP280 I2C sensor driver in src/drivers/sensors/BME280Sensor.hpp and src/drivers/sensors/BME280Sensor.cpp
- [x] T015c [P] [US2] Implement AHT20 I2C sensor driver in src/drivers/sensors/AHT20Sensor.hpp and src/drivers/sensors/AHT20Sensor.cpp
- [x] T015d [P] [US2] Implement generic CustomSensorDriver template for plugin sensors in src/drivers/sensors/CustomSensor.hpp and src/drivers/sensors/CustomSensor.cpp

- [x] T016 [P] [US2] Implement MOSFET AOD4184 PWM heater driver in src/drivers/actuators/MosfetActuator.hpp and src/drivers/actuators/MosfetActuator.cpp
- [x] T017 [P] [US2] Implement Fan driver (PWM / digital / shared MOSFET) in src/drivers/actuators/FanActuator.hpp and src/drivers/actuators/FanActuator.cpp
- [x] T017a [P] [US2] Implement SSR actuator driver in src/drivers/actuators/SSRActuator.hpp and src/drivers/actuators/SSRActuator.cpp
- [x] T017b [P] [US2] Implement Stepper actuator driver in src/drivers/actuators/StepperActuator.hpp and src/drivers/actuators/StepperActuator.cpp
- [x] T017c [P] [US2] Implement Servo actuator driver in src/drivers/actuators/ServoActuator.hpp and src/drivers/actuators/ServoActuator.cpp
- [x] T017d [P] [US2] Implement Generic GPIO actuator driver in src/drivers/actuators/GPIOActuator.hpp and src/drivers/actuators/GPIOActuator.cpp
- [x] T017e [P] [US2] Implement SharedMosfetActuator for heater+fan on same MOSFET in src/drivers/actuators/SharedMosfetActuator.hpp and src/drivers/actuators/SharedMosfetActuator.cpp

- [x] T018 [US2] Implement WebSocketServer handling control/start, control/stop, and config/hardware in src/network/WebSocketServer.hpp and src/network/WebSocketServer.cpp
- [x] T019 [US2] Implement 1Hz status update broadcast on topic status/update in src/network/WebSocketServer.cpp
- [x] T019a [US2] Implement generic hardware config WebSocket handler (config/hardware) with full JSON schema validation in src/network/WebSocketServer.cpp
- [x] T019b [US2] Implement HTTP GET/POST /api/hardware/config endpoints in src/network/WebServer.cpp
- [x] T019c [US2] Implement generic status payload builder reflecting all configured sensors/actuators in src/network/WebSocketServer.cpp

**Checkpoint**: User Story 2 complete and testable independently.

---

## Phase 5: User Story 3 - Fault Tolerance, Safety Engine & Fixed HTTP Logs (Priority: P1)

**Goal**: Immediate safe abort on fault (<50ms MOSFET cutoff) while maintaining platform/network, plus fixed HTTP log downloads (/log/system and /log/drying).

**Independent Test**: Disconnect sensor during cycle -> Heater PWM drops to 0% -> System state moves to FAULT_STOPPED -> Download log via GET /log/drying and verify fault reason.

- [x] T020 [US3] Implement thermal runaway and sensor timeout detection logic in src/core/SafetyEngine.cpp
- [x] T021 [US3] Implement safe abort emergency handler forcing PWM outputs LOW in src/core/SafetyEngine.cpp
- [x] T022 [P] [US3] Implement fixed HTTP endpoint GET /log/drying in src/network/WebServer.cpp
- [x] T023 [P] [US3] Implement fixed HTTP endpoint GET /log/system in src/network/WebServer.cpp
- [x] T024 [US3] Implement WebSocket real-time log streaming on topic logs/stream in src/network/WebSocketServer.cpp

**NEW: Generic Safety Framework**
- [x] T024a [US3] Implement configurable safety limits per actuator type in SafetyEngine (max_temp_c, max_power_pct, sensor_timeout_ms, thermal_runaway_time_sec, thermal_runaway_temp_rise_c) in src/core/SafetyEngine.hpp and src/core/SafetyEngine.cpp
- [x] T024b [US3] Implement sensor validation: timeout, range, rate-of-change checks per sensor capability in src/core/SafetyEngine.cpp
- [x] T024c [US3] Implement I2C bus lockup detection (SDA stuck LOW) and recovery sequence (9 clock cycles) in src/core/SafetyEngine.cpp
- [x] T024d [US3] Implement SPI bus error detection and recovery in src/core/SafetyEngine.cpp
- [x] T024e [US3] Implement actuator fault detection: PWM output mismatch, overcurrent detection in src/core/SafetyEngine.cpp
- [x] T024f [US3] Implement fault codes: SENSOR_DISCONNECT, OVER_TEMPERATURE, THERMAL_RUNAWAY, I2C_BUS_LOCKUP, SPI_BUS_ERROR, ACTUATOR_FAULT, NVS_CORRUPT, WATCHDOG_RESET in src/core/SafetyEngine.hpp

**Checkpoint**: User Story 3 complete and testable independently.

---

## Phase 6: User Story 4 - Generic Multi-Display Support & Auto-Layout Engine (Priority: P2)

**Goal**: Display telemetry and status on any attached display (ST7789, ILI9341, SSD1306, SH1106, ST7735, GC9A01, ILI9488, HD44780, Nextion) via LovyanGFX zero-copy DMA with auto-adapting layout engine.

**Independent Test**: Send config/display via WebSocket -> Display renders status fields at 1Hz without blocking control loop.

- [x] T025 [P] [US4] Implement ST7789 TFT display driver in src/drivers/display/ST7789Display.hpp and src/drivers/display/ST7789Display.cpp
- [x] T026 [P] [US4] Implement ILI9341 display driver for CYD board in src/drivers/display/ILI9341Display.cpp
- [x] T027 [P] [US4] Implement SSD1306 OLED display driver in src/drivers/display/SSD1306Display.cpp
- [x] T028 [US4] Implement auto-adapting display layout manager in src/drivers/display/DisplayManager.cpp

**NEW: Extended Display Drivers & Auto-Detection**
- [x] T028a [P] [US4] Implement SH1106 OLED display driver in src/drivers/display/SH1106Display.hpp and src/drivers/display/SH1106Display.cpp
- [x] T028b [P] [US4] Implement ST7735 TFT display driver in src/drivers/display/ST7735Display.hpp and src/drivers/display/ST7735Display.cpp
- [x] T028c [P] [US4] Implement GC9A01 round TFT display driver in src/drivers/display/GC9A01Display.hpp and src/drivers/display/GC9A01Display.cpp
- [x] T028d [P] [US4] Implement ILI9488 large TFT display driver in src/drivers/display/ILI9488Display.hpp and src/drivers/display/ILI9488Display.cpp
- [x] T028e [P] [US4] Implement HD44780 character LCD (I2C backpack) driver in src/drivers/display/HD44780Display.hpp and src/drivers/display/HD44780Display.cpp
- [x] T028f [P] [US4] Implement Nextion UART HMI display driver in src/drivers/display/NextionDisplay.hpp and src/drivers/display/NextionDisplay.cpp
- [x] T028g [US4] Implement DisplayManager auto-detection (try ST7789, ILI9341, SSD1306 in order) in src/drivers/display/DisplayManager.cpp
- [x] T028h [US4] Implement auto-layout engine: auto font scaling, field density adjustment per resolution, compact mode in src/drivers/display/DisplayManager.cpp
- [x] T028i [US4] Implement configurable display refresh rate (1Hz-5Hz) decoupled from status stream in src/drivers/display/DisplayManager.cpp

**Checkpoint**: User Story 4 complete and testable independently.

---

## Phase 7: User Story 5 - Filament Drying Profiles Management (Priority: P2)

**Goal**: Built-in material presets (PLA, PETG, ABS, TPU, Nylon) and custom profile management via WebSocket API.

**Independent Test**: Create custom profile via WebSocket -> Start session using profile ID -> Parameters applied to drying run.

- [x] T029 [P] [US5] Implement built-in material profiles in src/core/ProfileManager.hpp and src/core/ProfileManager.cpp
- [x] T030 [US5] Implement custom profile NVS persistence and WebSocket API handlers in src/network/WebSocketServer.cpp

**Checkpoint**: User Story 5 complete and testable independently.

---

## Phase 8: User Story 6 - Automated PID Calibration Routine (Priority: P2)

**Goal**: Implement Ziegler-Nichols PID auto-tuning (`control/pid_calibrate`) to calculate optimal `Kp`, `Ki`, `Kd` for heater surfaces and save persistently in NVS.

**Independent Test**: Send `control/pid_calibrate` via WebSocket -> Heater cycles PWM to measure thermal response -> Calculated PID coefficients streamed on `status/pid_calibrate` and saved to NVS.

- [x] T031 [P] [US6] Implement PidAutotuneController Ziegler-Nichols calculation engine in src/core/PidAutotuneController.hpp and src/core/PidAutotuneController.cpp
- [x] T032 [US6] Implement WebSocket handlers for control/pid_calibrate and status/pid_calibrate progress stream in src/network/WebSocketServer.cpp
- [x] T033 [US6] Save calibrated Kp, Ki, Kd coefficients persistently in NVS under heater.pid in src/core/ConfigManager.cpp

**NEW: Generic Auto-Tune for Any Control Algorithm**
- [x] T033a [P] [US6] Extend PidAutotuneController to support Bang-Bang and Feedforward algorithm auto-tune in src/core/PidAutotuneController.cpp
- [x] T033b [US6] Implement algorithm-agnostic auto-tune interface in IControlAlgorithm for custom algorithms in src/core/IControlAlgorithm.hpp

**Checkpoint**: User Story 6 complete and testable independently.

---

## Phase 9: User Story 7 - Generic Control Algorithm Framework (Priority: P2)

**Goal**: Support multiple control algorithms (PID, Bang-Bang, PWM Feedforward, Custom) selectable via configuration, with auto-tune support.

**Independent Test**: Configure different algorithms via config/control -> Verify each regulates temperature correctly -> Auto-tune works for each.

- [x] T034 [P] [US7] Implement BangBangControl algorithm (hysteresis-based) in src/control/BangBangControl.hpp and src/control/BangBangControl.cpp
- [x] T035 [P] [US7] Implement PWMFeedforwardControl algorithm (base PWM + temp coefficient) in src/control/PWMFeedforwardControl.hpp and src/control/PWMFeedforwardControl.cpp
- [x] T036 [P] [US7] Implement IControlAlgorithm interface and ControlEngine for algorithm switching in src/control/IControlAlgorithm.hpp and src/control/ControlEngine.hpp/.cpp
- [x] T037 [US7] Register control algorithms in DriverRegistry and integrate with ConfigManager in src/core/ConfigManager.cpp
- [x] T038 [US7] Implement config/control WebSocket handler for algorithm selection and parameter configuration in src/network/WebSocketServer.cpp

**Checkpoint**: User Story 7 complete and testable independently.

---

**Checkpoint**: User Story 7 complete and testable independently.

---

## Phase 10: User Story 8 - Generic Sensor/Actuator Calibration & Advanced Features (Priority: P2)

**Goal**: Support per-capability calibration (offset/scale), sensor validation, actuator fault detection, and bus recovery.

**Independent Test**: Configure sensor with offset/scale -> Verify readings adjusted -> Disconnect sensor -> Fault detected and logged -> Reconnect -> Auto-recovery.

- [x] T039 [P] [US8] Implement sensor calibration (offset/scale per capability) in ConfigManager and apply in sensor drivers in src/core/ConfigManager.cpp and sensor drivers
- [x] T040 [P] [US8] Implement sensor validation: timeout, range check, rate-of-change per capability in src/core/SafetyEngine.cpp
- [x] T041 [US8] Implement actuator fault detection: PWM output verification, overcurrent sense in src/core/SafetyEngine.cpp and actuator drivers
- [x] T042 [US8] Implement I2C bus recovery (9 clock cycles) and SPI error recovery in src/core/SafetyEngine.cpp
- [x] T043 [US8] Implement WebSocket fault notification (status/fault) with standardized fault codes in src/network/WebSocketServer.cpp
- [x] T044 [P] [US8] Add calibration fields to sensor config schema and validate in HardwareConfigParser in src/core/HardwareConfigParser.cpp

**Checkpoint**: User Story 8 complete and testable independently.

---

## Phase 11: Polish & Cross-Cutting Concerns

**Purpose**: Documentation, memory optimization, and quickstart validation.

- [x] T045 [P] Update firmware documentation and API references in README.md
- [x] T046 Perform memory leak profiling and FreeRTOS stack verification
- [x] T047 Execute end-to-end quickstart validation scenarios in specs/001-filament-dryer-esp32/quickstart.md

---

## Dependencies & Execution Order

1. **Phase 1 (Setup)** -> Blocks Phase 2
2. **Phase 2 (Foundational + Driver Registry + Hardware Config Parser)** -> Blocks all User Stories
3. **Phase 3 (US1)** -> Priority P1 (MVP)
4. **Phase 4 (US2)** -> Priority P1
5. **Phase 5 (US3)** -> Priority P1
6. **Phase 6 (US4)** -> Priority P2
7. **Phase 7 (US5)** -> Priority P2
8. **Phase 8 (US6)** -> Priority P2
9. **Phase 9 (US7)** -> Priority P2
10. **Phase 10 (US8)** -> Priority P2
11. **Phase 11 (Polish)** -> Final

---

## Implementation Strategy

### MVP First (Phases 1-3)
Complete Setup, Foundational, and User Story 1 (WiFi Setup Hotspot) to produce a bootable, configurable hardware node.

### Incremental Delivery
Add US2 (Control & Telemetry) -> Add US3 (Safety Engine & Logs) -> Add US4 (Displays) -> Add US5 (Profiles) -> Add US6 (PID Calibration) -> Add US7 (Generic Control Algorithms) -> Add US8 (Calibration & Advanced Safety).

## Phase 12: Convergence

- [x] T048 Fix the current PlatformIO build failures in the core configuration, logging, and PID modules by correcting const-correctness issues, adding required Arduino/ESP headers, and resolving the remaining compile errors per plan: foundational infrastructure (partial)
- [x] T049 Repair the firmware initialization wiring in src/main.cpp so the WebSocket/WebServer startup uses the correct dependency objects and signatures per FR-002 (partial)
- [x] T050 Remove or refactor the duplicate helper module in src/helpers.cpp so it no longer includes src/main.cpp directly and causes conflicting translation-unit definitions per plan: main firmware entrypoint (partial)

## Phase 13: Convergence

- [x] T051 Add PlatformIO Unity test infrastructure (native or host test env) and implement test suites in test/test_state_machine/, test/test_safety_engine/, test/test_log_manager/, and test/test_config_parser/ so `pio test` runs and passes per plan: testing structure and Constitution: Teste Automatizado e Qualidade (missing)
- [x] T052 Wire WebSocketServer::dispatchTopic to all FR-002 topics and implement control/start, control/stop, config/hardware, config/control, config/display, config/profiles, and control/pid_calibrate handlers instead of no-op stubs in src/network/WebSocketServer.cpp per FR-002 and FR-005 (partial)
- [x] T053 Implement ProfileManager with built-in material presets and custom profile NVS persistence in src/core/ProfileManager.hpp and src/core/ProfileManager.cpp per FR-008 and T029 (missing)
- [x] T054 Re-enable src/core/DriverRegistry.cpp in the firmware build, implement DriverRegistry::registerBuiltins() with all built-in driver factories, and replace hardcoded driver instances in src/main.cpp with generic config-driven loading per FR-003 (partial)
- [x] T055 Fix compile errors in sensor, actuator, and display drivers currently excluded via build_src_filter in platformio.ini and remove those exclusions so the full driver catalog builds per FR-003 and US2/US4 (partial)
- [x] T056 Re-enable src/drivers/display/DisplayManager.cpp in the build and complete auto-detection, auto-layout, and configurable refresh-rate behavior per FR-004 and T028 (partial)
- [x] T057 Implement GET/POST /api/hardware/config HTTP endpoints in src/network/WebServer.cpp (register routes in setupRoutes and replace unsupported stubs) per FR-003 and T019b (partial)
- [x] T058 Add a GitHub Actions CI workflow that runs `pio run` and `pio test` on pull requests and fails the build when tests fail per Constitution: Teste Automatizado e Qualidade (missing)
- [x] T059 Integrate ControlEngine and config/control WebSocket handling into the firmware runtime (main loop and WebSocketServer) so algorithm selection and parameters apply during drying per FR-010 and US7 (partial)

## Phase 14: Convergence

- [x] T060 CRITICAL Attach the AsyncWebSocket instance to the AsyncWebServer via addHandler so clients can connect to ws://<ip>/ws in src/network/WebServer.cpp and src/network/WebSocketServer.cpp per FR-002 (missing)
- [x] T061 CRITICAL Wire POST /api/wifi/config (and the captive portal submit path) to WifiManager::setConfig so credentials trigger STA connection and AP disable without requiring a reboot in src/network/WebServer.cpp and src/main.cpp per FR-001 and US1 (partial)
- [x] T062 CRITICAL Add PlatformIO Unity end-to-end flow tests covering P1 user journeys (hotspot WiFi setup, WebSocket start/stop/status, sensor-fault safe abort) under test/ per Constitution: Teste Automatizado e Qualidade (missing)
- [x] T063 Implement a functional captive portal HTML page with SSID/password form (PT/EN) posting to /api/wifi/config in src/network/WebServer.cpp per FR-001 and US1 (partial)
- [x] T064 Complete SafetyEngine FR-011 features: sensor range and rate-of-change checks, I2C/SPI bus lockup detection and recovery, actuator fault detection, and missing fault codes (SPI_BUS_ERROR, ACTUATOR_FAULT) in src/core/SafetyEngine.hpp/.cpp per FR-011 and US8 (partial)
- [x] T065 Apply per-capability sensor calibration (offset/scale) from config when producing SensorReading values in sensor drivers per FR-003 and US8 (missing)
- [x] T066 Implement the 30s exhaust-fan cooldown after humidity-reached or normal cycle completion before fully stopping actuators in src/main.cpp per spec clarification Session 2026-07-23 (missing)
- [x] T067 Fix compile issues in src/plugins/PluginManager.cpp, remove its build_src_filter exclusion, and wire PluginManager hooks into firmware init/runtime per plan: plugin architecture (partial)
- [x] T068 Create and maintain .vscode/workspace.json with the PlatformIO toolchain and documented dependencies per Constitution: Workspace de Dependências e Configurações (missing)

## Phase 15: Convergence

- [x] T069 CRITICAL Cut heater/fan power on control/stop and whenever leaving DRYING without cooldown in src/network/WebSocketServer.cpp and src/main.cpp per FR-005 and Constitution: Failsafe e Proteção de Hardware (missing)
- [x] T070 CRITICAL Implement SafetyEngine::executeEmergencyShutdown actuator cutoff, attach control task to WDT via esp_task_wdt_add, and invoke I2C/SPI/actuator fault checks from the control loop in src/core/SafetyEngine.cpp and src/main.cpp per FR-011 and Constitution: Failsafe e Proteção de Hardware (partial)
- [x] T071 CRITICAL Populate ActuatorConfig pins, types, PWM freqs, cooldown, and safety limits from the actuators JSON array in HardwareConfigParser::parseActuators in src/core/HardwareConfigParser.cpp per FR-003 (missing)
- [x] T072 CRITICAL Instantiate sensors and actuators from persisted config types via DriverRegistry instead of hardcoding mosfet_pwm + fan_pwm in src/main.cpp per FR-003 (partial)
- [x] T073 CRITICAL Register PidAutotune progress/complete callbacks that broadcast status/pid_calibrate and persist Kp/Ki/Kd to NVS in src/network/WebSocketServer.cpp and src/main.cpp per FR-009 (missing)
- [x] T074 CRITICAL Feed live chamber_temp_c and humidity_pct from sensors into the display task instead of session targets in src/main.cpp per FR-007 (contradicts)
- [x] T075 Return WiFi connect success only when STA is up and report HTTP/WS status accordingly in src/network/WifiManager.cpp and src/network/WebServer.cpp per FR-001 (partial)
- [x] T076 Build status/update from live configured sensors/actuators/metrics and send only to status/subscribe clients in src/network/WebSocketServer.cpp and src/main.cpp per FR-006 (partial)
- [x] T077 Tear down previous drivers on hardware reload and invoke the same reload path from HTTP POST /api/hardware/config in src/main.cpp and src/network/WebServer.cpp per FR-003 (partial)
- [x] T078 Register captive-portal / DNS redirect handlers so AP clients reach the WiFi config page in src/network/WebServer.cpp per FR-001 (partial)
- [x] T079 Enforce max 20 custom profiles and validate temp 30–80°C, duration 1–1440 min, humidity 5–50% on create/update in src/core/ConfigManager.cpp and ProfileManager per FR-008 (missing)
- [x] T080 Apply config/display fields and refresh_rate_hz into DisplayManager layout in src/network/WebSocketServer.cpp per FR-004 (partial)
- [x] T081 Invoke PluginManager session/telemetry hooks (e.g. callOnSessionStart) from drying start/runtime in src/main.cpp per plan: plugin architecture (partial)
- [x] T082 Expand native P1 flow tests to exercise WebSocket start/stop/status and fault-abort handler paths beyond in-memory stubs in test/test_p1_flows/ per Constitution: Teste Automatizado e Qualidade (partial)

## Phase 16: Convergence

- [x] T083 CRITICAL Replace blocking WiFi STA connect (delay loop up to 20s) with non-blocking connection driven by WifiManager state/loop in src/network/WifiManager.cpp per Constitution: Desempenho Máximo e Eficiência (contradicts)
- [x] T084 Apply parsed control/hardware safety_limits into SafetyEngine on boot and reload and use that limit in the drying loop instead of hardcoded 80C in src/main.cpp per FR-005 and FR-011 (partial)
- [x] T085 Persist and instantiate multiple sensors by capability (separate temp/humidity) and reflect all configured sensors in status/update in src/core/HardwareConfigParser.cpp and src/main.cpp per FR-003 and FR-006 (partial)
- [x] T086 Implement FR-008 WebSocket topics config/profiles/list|get|create|update|delete|reset_defaults (with auto UUID on create) in src/network/WebSocketServer.cpp per FR-008 (contradicts)
- [x] T087 Allow custom profiles to override builtin IDs per FR-008 shadow/override rules in src/core/ConfigManager.cpp (contradicts)
- [x] T088 Map nested display bus and geometry fields into DisplayConfig in HardwareConfigParser::parseDisplay in src/core/HardwareConfigParser.cpp per FR-003 and FR-004 (partial)
- [x] T089 Wire real actuator fault feedback or overcurrent sensing and invoke SPI bus recovery from SPI driver failures in src/main.cpp and SafetyEngine per FR-011 (partial)
- [x] T090 Add DNS sinkhole/captive redirect to AP IP while hotspot is active in src/network/WebServer.cpp or WifiManager per FR-001 (partial)
- [x] T091 Validate control/start explicit parameters with profile range rules (temp/duration/humidity) in src/core/ProfileManager.cpp per FR-008 (partial)
- [x] T092 Materialize custom-role actuators and fix shared_mosfet to a single shared driver instance in src/main.cpp per FR-003 (partial)
- [x] T093 Align status/fault and pid_calibrate payloads with contracts/websocket-api.md (fault_code string, cycles alias) in src/network/WebSocketServer.cpp per plan: websocket contract (contradicts)
- [x] T094 Add host-testable WebSocket/HTTP handler-path coverage beyond domain stubs in test/test_p1_flows/ per Constitution: Teste Automatizado e Qualidade (partial)
- [x] T095 Create docs/PT-BR and docs/EN-US API/architecture documentation trees referenced by README and Constitution Language Support (missing)

## Phase 17: Convergence

- [x] T096 CRITICAL Add GPLv3 license headers to all firmware sources under src/ and include/ per Constitution: Governance GPLv3 (missing)
- [x] T097 CRITICAL Defer marking drying status as STOPPED until post-completion exhaust-fan cooldown finishes (add COOLDOWN state or delay stopDrying) in src/core/StateMachine and src/main.cpp per FR-005 clarification Session 2026-07-23 (contradicts)
- [x] T098 Implement real GPIO/bus pin-conflict detection in HardwareConfigParser::checkPinConflicts instead of the empty stub in src/core/HardwareConfigParser.cpp per FR-003 (missing)
- [x] T099 Pause ControlEngine heater writes while PidAutotuneController is running so calibration and drying control do not contend for the heater in src/main.cpp per FR-009 (partial)
- [x] T100 Create and seed the shared memory catalog required by Constitution Memória Compartilhada with research and architecture decisions from specs/001-filament-dryer-esp32/research.md (missing)
- [x] T101 Implement GET /api/info returning firmware_version, chip_model, mac_address, free_heap_bytes, and system_status in src/network/WebServer.cpp per plan: http-api contract (contradicts)
- [x] T102 Honor SafetyConfig::sensor_timeout_ms for sensor-disconnect detection instead of the fixed fail-count heuristic in src/core/SafetyEngine.cpp per FR-011 (partial)
- [x] T103 Sync README.md progress, constitution badge, and WebSocket/API descriptions with the actual firmware (port 80 /ws, real topics, non-zero firmware progress) per Constitution: Documentação Sincronizada (contradicts)
- [x] T104 Eliminate blocking delay() from sensor read paths used by the 50Hz control loop (non-blocking/async reads) in src/drivers/sensors/ per Constitution: Desempenho Máximo e Eficiência and SC-08 (contradicts)
- [x] T105 Tear down and reinitialize display drivers on hardware-config hot-reload in src/main.cpp and DisplayManager per FR-003 (partial)
- [x] T106 Propagate DisplayManager font_scaling and compact_mode into driver render payloads and pixel text sizing per FR-004 (partial)
- [x] T107 Pass the same status-stream fields (elapsed, remaining, cpu, memory, uptime, actuator states) into the display-task payload per FR-007 (partial)
- [x] T108 Unify WiFi credential NVS read/write ownership between ConfigManager and WifiManager to prevent dual-writer conflicts per FR-001 (partial)
- [x] T109 Register a fan_digital actuator factory (or reject the type at parse) matching HardwareConfigParser allowed actuator types per FR-003 (partial)
- [x] T110 Fail-fast STA connect so hotspot activation meets the SC-01 <5s target when credentials are missing or invalid in src/network/WifiManager.cpp per SC-01 (partial)
- [x] T111 Remove unrequested stale *.bak source copies under src/ per unrequested cleanup (unrequested)
- [x] T112 Reconcile partitions.csv dual-OTA layout with spec OTA out-of-scope (single-app partition or documented intentional reserve) per plan: out-of-scope OTA (unrequested)

## Phase 18: Convergence

- [x] T113 CRITICAL Sync README.md progress (Firmware Core), WebSocket topic list (drop phantom `config/sensors`), and repository tree to match the live firmware per Constitution: Documentação Sincronizada (contradicts)
- [x] T114 CRITICAL Replace blocking `delay(2000)` splash waits (and related init delays) in display drivers under src/drivers/display/ with non-blocking millis-based timing per Constitution: Desempenho Máximo e Eficiência (contradicts)
- [x] T115 CRITICAL Honor sensor disconnect timeout (`sensor_timeout_ms`) during drying instead of immediately aborting on first invalid reading in src/main.cpp so status reports error and drying continues under SafetyEngine limits per Edge Case sensor read failure and FR-011 (contradicts)
- [x] T116 Wire ControlEngine so `algorithm: "custom"` resolves via registered plugin/DriverRegistry factory, or reject `custom` at parse until then, in src/control/ControlEngine.cpp and HardwareConfigParser per FR-010 (missing)
- [x] T117 Implement real SPI bus recovery (re-init bus/display) in SafetyEngine::detectAndRecoverSpiBusError and invoke it from SPI driver failure paths before faulting per FR-011 (missing)
- [x] T118 Implement real actuator fault feedback (current sense or open-loop detect) so ACTUATOR_FAULT is not stubbed via commanded==measured and e-stop-as-overcurrent in SafetyEngine and actuator drivers per FR-011 (partial)
- [x] T119 Map per-actuator `safety_limits` (at least `max_temp_c` / max power) into SafetyEngine on boot and reload, and enforce max heater power in the safety path, in HardwareConfigParser and main.cpp per FR-005 and FR-011 (partial)
- [x] T120 Expand docs/PT-BR and docs/EN-US with full API payload examples, architecture detail, and configuration docs required by Constitution Language Support (partial)
- [x] T121 Broadcast 1Hz `status/update` during COOLDOWN (heater 0%, fan on, status COOLDOWN) instead of skipping `broadcastTelemetry` in src/main.cpp per FR-005 and FR-006 (partial)
- [x] T122 Call `displayManager.end()` on hardware hot-reload when display is disabled or removed in src/main.cpp teardown/initializeDisplays per FR-003 and FR-004 (partial)
- [x] T123 Persist and apply `layout.font_scaling` through DisplayConfig, HardwareConfigParser, and config/display handling per FR-004 (missing)
- [x] T124 Align DisplayManager auto-layout field names with the status stream (`memory_free_bytes` not `free_heap_bytes`) and include uptime/actuator boolean fields in default field sets per FR-007 (contradicts)
- [x] T125 After `config/control` persist, refresh SafetyEngine from `control.safety_limits` (same path as hardware reload) in WebSocketServer/main.cpp per FR-005 (partial)
- [x] T126 Persist I2C display bus pins/address, include them in pin/bus conflict checks, and detect I2C address clashes in HardwareConfigParser per FR-003 (partial)
- [x] T127 Apply PID autotune results to the current algorithm when it exposes PID-like params instead of always forcing `setAlgorithm("pid")` in src/main.cpp per FR-010 (partial)
- [x] T128 Collapse `config/profiles/list` so an overridden builtin ID appears once (custom shadows builtin) in ConfigManager/WebSocketServer per FR-008 (partial)
- [x] T129 Dispatch unmatched WebSocket topics and HTTP paths through PluginManager extension hooks in WebSocketServer/WebServer per plan: plugin architecture (partial)
- [x] T130 Allow `control/stop` during COOLDOWN to cancel cooldown, cut fan/heater, and mark STOPPED in WebSocketServer/main.cpp per FR-005 (partial)
- [x] T131 Parse/persist fan `speed_curve` and use it when commanding fan power instead of a fixed duty in FanActuator/main.cpp per FR-003 (missing)
- [x] T132 Align HardwareConfigParser allowed sensor type identifiers with DriverRegistry factories (aliases or reject unknowns; include `sht31`) per FR-003 (partial)
- [x] T133 Align `status/pid_calibrate` completion payload with contract (`status: "complete"`, set `saved_to_nvs` only after successful NVS write) in src/main.cpp per FR-009 and plan: websocket contract (contradicts)
- [x] T134 Use a sensor-oriented fault code for rate-of-change validation failures instead of ACTUATOR_FAULT in SafetyEngine::validateSensorReading per FR-011 (partial)

## Phase 19: Convergence

> **Anti-regression (read before implement/converge):** Open Phase 18 tasks **T113–T134** already track prior gaps — do **not** re-list or re-implement them under new IDs. Mark any Phase 18/19 task `[x]` only when **all Done-When** criteria pass. Next `/speckit-converge` must treat unchecked open tasks as in-progress coverage (not new findings). Gaps below are **net-new** only.

### Phase 18 Done-When reference (do not create new IDs)

- **T113**: README Firmware Core progress reflects implemented firmware (not 0%/AGUARDANDO); topic list has no phantom `config/sensors`; repo tree matches live `src/`/`test/`/`docs/`.
- **T114**: No `delay(` splash/init waits on display driver begin paths; splash uses millis state or is removed from critical path.
- **T115**: First invalid sensor sample does **not** call `stopDrying`; heater limited/cut only after `sensor_timeout_ms`; status can report sensor error while session continues until timeout/fault.
- **T116**: `algorithm:"custom"` either loads via registered factory **or** is rejected at parse with clear error (no silent accept-then-fail).
- **T117**: `detectAndRecoverSpiBusError` attempts bus/display re-init; faults only if recovery fails; called from SPI I/O failure paths.
- **T118**: `ACTUATOR_FAULT` requires real feedback (sense pin / open-loop heuristic that can trip); commanded==measured alone must not be the only check; e-stop must not count as overcurrent.
- **T119**: Actuator `safety_limits.max_temp_c` / max power applied into `SafetyEngine` on boot+reload; safety path enforces max heater power.
- **T120**: `docs/PT-BR` + `docs/EN-US` include API payloads/examples, architecture, and configuration (not stubs).
- **T121**: During COOLDOWN, subscribed clients receive 1Hz `status/update` with heater 0%, fan on, status reflecting cooldown.
- **T122**: `enabled:false` or missing display on reload calls `displayManager.end()`; no stale driver left active.
- **T123**: `font_scaling` round-trips via DisplayConfig NVS + `config/display` + hardware layout parse into DisplayManager.
- **T124**: Auto-layout uses `memory_free_bytes` (not `free_heap_bytes`); default fields include uptime + actuator on/off where resolution allows.
- **T125**: After `config/control` save, `SafetyEngine` limits match payload without requiring hardware reload/reboot.
- **T126**: I2C display SDA/SCL/address persisted, claimed in pin conflicts, address clashes detected.
- **T127**: Autotune success updates current algo PID-like params when applicable; does not unconditionally `setAlgorithm("pid")`.
- **T128**: `config/profiles/list` returns one effective profile per ID when custom shadows builtin.
- **T129**: Unmatched WS topics / HTTP paths invoke PluginManager hooks when registered.
- **T130**: `control/stop` in COOLDOWN cancels cooldown, cuts actuators, reaches STOPPED.
- **T131**: Fan `speed_curve` parsed/persisted and used for commanded fan power when present.
- **T132**: Parser allowed sensor types ≡ DriverRegistry factories (aliases or reject); `sht31` accepted if registered.
- **T133**: Completion broadcast `status:"complete"`; `saved_to_nvs:true` only after successful NVS write.
- **T134**: Excessive RoC uses sensor-oriented fault code (not `ACTUATOR_FAULT`).

### Net-new gaps

- [x] T135 CRITICAL Reject `config/hardware` and HTTP `POST /api/hardware/config` reload while state is DRYING or COOLDOWN (return clear error; do not call `teardownDrivers`), or emergency-stop then reload — Done-When: reload during drying never tears down live heaters without e-stop; covered by native/flow assertion — per FR-003 hot-reload-where-possible and FR-011 / Constitution: Failsafe (missing)
- [x] T136 Align WebSocket envelope with `contracts/websocket-api.md`: `sendError` payload key `error` (not only `message`); `config/hardware` ack topic `config/hardware/response` with `status:"saved"` — Done-When: contract examples match captured WS frames for error + hardware save — per plan: websocket contract (contradicts)
- [x] T137 Emit `status/update.status` as contract/data-model lowercase values (`drying`, `cooldown`, `stopped`, `fault_stopped`, `ready`/`idle` as documented) instead of raw `DRYING`/`READY` enums — Done-When: status stream + display status field use the same lowercase vocabulary; COOLDOWN included — per FR-006 and plan: websocket/data-model contract (contradicts)
- [x] T138 Make `GET /api/hardware/config` round-trip the same Klipper-style schema as WS `config/hardware` (`sensors[]`, `actuators[]`, nested display bus/geometry/layout, control) instead of legacy flat `sensor`/`actuator` — Done-When: GET body can be POSTed back successfully — per plan: http-api contract and FR-003 (contradicts)
- [x] T139 Align HTTP WiFi/info/log contract: `POST /api/wifi/config` accepts form **or JSON** and returns `{status,message}` per http-api; `GET /api/info` includes `active_feature`; log downloads send `text/plain; charset=utf-8` + `Content-Disposition: attachment` — Done-When: responses match `contracts/http-api.md` examples — per plan: http-api contract (partial)
- [x] T140 CRITICAL Fix `SystemMetrics::update` so `cpu_usage_pct` is not ~0 (enable/use FreeRTOS run-time stats or idle-task sampling); expose non-zero under load in status stream — Done-When: host or firmware test shows cpu_usage_pct responds to load; heap path unchanged — per FR-006 NFR metrics and T010 false-complete (contradicts)
- [x] T141 Reject ESP32 input-only GPIOs 34–39 (and other non-output-capable pins) for actuator PWM/digital outputs and display CS/DC/RST/BL in `HardwareConfigParser::isValidGPIOPin` / actuator validation — Done-When: config with heater PWM on 34 fails validation with clear error; valid output pins still accepted — per FR-003 valid GPIO ranges (missing)
- [x] T142 Add instrumented native/flow tests asserting Success Criteria timing bounds: hotspot/AP-ready fail-fast &lt;5s (SC-01), safety cutoff sensor→heater-off &lt;100ms (SC safety), status interval 1Hz ±100ms — Done-When: `pio test -e native` fails if bounds violated — per Spec Success Criteria and Constitution: Teste Automatizado (missing)
- [x] T143 Emit `logs/stream` with `target_log: "drying"|"system"` per websocket contract (keep `drying` bool only as optional alias) — Done-When: contract §2.3 field present on every log frame — per plan: websocket contract (contradicts)
- [x] T144 Align profile WS payloads with contract: get wraps `{profile:{…}}`; create/update ack `{status:"created"|"updated", profile_id}`; list/get include `created_at`/`updated_at` for custom profiles — Done-When: frames match `contracts/websocket-api.md` §2.7 — per FR-008 (partial)
- [x] T145 Persist and apply `layout.compact_mode` through DisplayConfig + HardwareConfigParser + `config/display` (alongside T123 font_scaling) — Done-When: compact_mode survives reboot and affects render payload — per FR-004 and data-model layout (missing)
- [x] T146 Resolve research/DEC-003 `triac` and display `parallel_8bit`: either register factories + parser support, or reject unknown types at parse and update shared-memory catalog/DEC-003 to match shipped matrix — Done-When: no accepted-but-unloadable type remains — per FR-003 and Constitution: Memória Compartilhada (partial)
- [x] T147 Assign monotonic `DryingSession.session_id` (&gt;0) on `startDrying` and include `session_id` + `stop_reason` on `status/update` when a session is active/recent — Done-When: data-model fields appear in telemetry; session_id increments per start — per plan: data-model and FR-006 (missing)
- [x] T148 Make captive-portal detection endpoints (`/generate_204`, `/fwlink`, `/hotspot-detect.html`, `/ncsi.txt`, `/connecttest.txt`) **redirect** to `/` per http-api §3 instead of returning 200 HTML inline — Done-When: those routes respond with redirect to captive root — per FR-001 and plan: http-api contract (partial)

---

## Phase 20: Developer Tooling — Root Makefile (plan add-on 2026-07-28)

**Purpose**: Host-side GNU Make façade over PlatformIO for compile, native tests, and ESP32 flash/install (DEC-009 / `contracts/makefile-targets.md`). No firmware runtime API change.

**Independent Test**: `make help` lists targets; `make test` runs `pio test -e native` and fails non-zero on failure; `make build` compiles default `ENV=esp32devkitc`; `make flash`/`upload`/`install` are aliases wrapping `pio run -e $(ENV) -t upload` (device required for flash success).

**Prerequisites**: `platformio.ini` envs `esp32devkitc`, `lilygo_tdisplay_v1`, `esp32_2432s028`, `native` already exist; `pio` on `PATH`.

- [x] T149 Create root `Makefile` with `.PHONY`, default `ENV=esp32devkitc`, optional `PORT`/`UPLOAD_PORT` passthrough, and `help` target documenting allowed ENVs per `contracts/makefile-targets.md` in Makefile
- [x] T150 [P] Implement `build` / `compile` / default goal targets as `pio run -e $(ENV)` in Makefile
- [x] T151 [P] Implement `test` target as `pio test -e native` (must propagate non-zero exit) in Makefile
- [x] T152 Implement equivalent `flash`, `upload`, and `install` targets as `pio run -e $(ENV) -t upload` with port flags when `PORT`/`UPLOAD_PORT` set in Makefile
- [x] T153 [P] Implement `uploadfs`, `monitor`, and `clean` targets per `contracts/makefile-targets.md` in Makefile
- [x] T154 [P] Sync developer docs to prefer Make: `README.md`, `specs/001-filament-dryer-esp32/quickstart.md`, `docs/PT-BR/configuracao.md`, `docs/EN-US/configuration.md`
- [x] T155 Confirm `.vscode/workspace.json` philarmony `build_command`/`test_command`/`flash_command` match Makefile targets (update if drift) in `.vscode/workspace.json`
- [x] T156 Wire CI to `make test` (and optionally `make build`) so PR checks fail when Make/pio tests fail in `.github/workflows/` per Constitution: Teste Automatizado e Qualidade
- [x] T157 Validate Done-When: `make help`, `make test`, and `make build` succeed locally; document flash requires attached ESP32 in `specs/001-filament-dryer-esp32/quickstart.md`

**Checkpoint**: Contributors can build, test, and flash via Make without memorizing `pio -e` flags.

### Phase 20 Done-When

- **T149–T153**: Root `Makefile` exists; targets match `contracts/makefile-targets.md`; `flash`≡`upload`≡`install`.
- **T154–T155**: Docs and workspace point at `make build|test|flash`.
- **T156**: CI invokes `make test` (or equivalent) and fails on non-zero.
- **T157**: Manual/host validation recorded in quickstart checklist.

---

## Dependencies & Execution Order (Phase 20)

1. **T149** → blocks T150–T153 (skeleton first)
2. **T150**, **T151**, **T153** can run in parallel after T149
3. **T152** after T149 (may share upload-port helper with T153)
4. **T154**, **T155** [P] after Makefile targets exist
5. **T156** after `make test` works
6. **T157** last (validation)

### Parallel example

```text
# After T149:
T150 + T151 + T153 in parallel
Then T152, then T154 + T155, then T156, then T157
```

### MVP (tooling)

`T149` + `T150` + `T151` + `T152` + `T157` — enough to compile, test, and flash.

### Implementation strategy

Do **not** re-open Phase 1–19 completed IDs. Implement Phase 20 only; keep wrapping `pio` (no alternate toolchain).

## Phase 21: Convergence

> **Anti-regression:** Do not re-open closed IDs T113–T157. Tasks below are **net-new** coverage for Done-When that still fails (T118/T133/T142) or newly confirmed gaps.

- [x] T158 CRITICAL Wire real actuator fault feedback end-to-end: use `hasOpenLoopFeedback()` (not only `hasCurrentSense()`), ensure `open_loop_detect` can trip mismatch (commanded==measured alone must not be the only path), and cover non-MOSFET heaters where applicable in `src/main.cpp` and `MosfetActuator` per FR-011 / Constitution: Failsafe and T118 Done-When (partial)
- [x] T159 Fail closed on DriverRegistry create failures in `initializeSensors`/`initializeActuators`: remove silent fallbacks to `sht3x`/`mosfet_pwm`/`fan_pwm`; log and leave drivers unset (or fault) when the configured type cannot be created per FR-003 (partial)
- [x] T160 Make `ConfigManager::setPidConfig` return NVS write success and set `saved_to_nvs` in `onPidCalibrateComplete` only when that write succeeds per FR-009 and T133 Done-When (contradicts)
- [x] T161 Implement Bang-Bang and PWM-Feedforward auto-tune paths in `PidAutotuneController` (or algorithm-specific tune via `IControlAlgorithm`) so `control/pid_calibrate` is not PID-only per FR-009 and T033a (missing)
- [x] T162 Route display instantiation through `DriverRegistry::createDisplay` (or Document why DisplayManager owns fixed members) so config-driven display types match the sensor/actuator registry path in `DisplayManager`/`main.cpp` per FR-003 (partial)
- [x] T163 Replace tautological SC timing tests with instrumented assertions that can fail: WifiManager fail-fast &lt;5s (SC-01), and a measurable 1Hz±100ms status period proxy — `test_sc_status_interval_1hz_within_100ms` must not assert literal `1000`/`50` constants per Spec Success Criteria and T142 Done-When (partial)
- [x] T164 Align Makefile default goal with `contracts/makefile-targets.md`: bare `make` builds firmware (`all`/`build`) while keeping `make help` available per plan: Makefile Target Map (contradicts)
- [x] T165 Stop silently remapping `fan_digital` to `fan_pwm` in `initializeActuators`; use the registered `fan_digital` factory (or reject at parse) per FR-003 (partial)

## Phase 22: Convergence

> **Scope note:** Firmware FR-001–FR-011 / Phase 21 Done-When verified satisfied. Only remaining actionable gap is living-README progress (no further firmware correction loop intended).

- [x] T166 CRITICAL Sync README.md project progress for Firmware Core (replace `AGUARDANDO` / `0%` with implemented status, and align feature `001` roadmap row with live firmware/WS/Make/CI) per Constitution: Documentação Sincronizada and T113 Done-When (contradicts)
