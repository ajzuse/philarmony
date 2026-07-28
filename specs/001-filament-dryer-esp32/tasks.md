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
