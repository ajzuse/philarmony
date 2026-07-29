# Technical Research & Architecture Decisions: Filament Dryer ESP32

## 1. Programming Language & Core Framework Selection

### Decision
**C++17 compiled with ESP-IDF v5.1+ (using Arduino Core as a component)**.

### Rationale
- **Performance & Determinism**: Native C++ compiled directly to Xtensa dual-core LX6/LX7 machine instructions provides maximum throughput and deterministic sub-millisecond execution for control loops.
- **Multitasking Isolation**: FreeRTOS dual-core task pinned execution:
  - **Core 0**: Network stack (WiFi, AsyncWebServer, WebSocket broadcast, HTTP Log streaming, LittleFS file I/O).
  - **Core 1**: Critical control loop (sensor acquisition, PID/PWM regulation, safety watchdog, display rendering).
- **Zero Dynamic Allocation**: Critical control loops use pre-allocated static buffers to prevent heap fragmentation (`FreeHeap` drops) during multi-day drying sessions.

### Alternatives Considered
- **Rust for ESP32 (`esp-rs`)**: High safety, but library support for diverse displays (LovyanGFX/Adafruit) and Async WebServers is less mature than C++.
- **MicroPython**: Easy development, but high garbage collection latency penalties (>50ms pauses) that breach the 20ms control loop requirement and consume excessive RAM (~2MB base runtime).

---

## 2. Generic Plugin-Driver Architecture (Klipper-Inspired)

### Core Philosophy
**"Everything is a Plugin"** - No hardcoded hardware references. All sensors, actuators, displays, control algorithms, and communication protocols are loaded dynamically from a driver registry based on JSON configuration.

### Architecture
```
                    +-------------------------+
                    |     ObjectRegistry      |
                    |  (Config → Drivers)     |
                    +-----------+-------------+
                                |
        +-----------------------+-----------------------+
        |                       |                       |
+-------v-------+       +-------v-------+       +-------v-------+
| ISensorDriver |       |IActuatorDriver|       |IDisplayDriver |
+-------+-------+       +-------+-------+       +-------+-------+
        |                       |                       |
+-------+-------+       +-------+-------+       +-------+-------+
| sht3x, dht22  |       |mosfet_pwm,    |       |st7789, ili9341|
| ds18b20, ntc  |       | fan_pwm,      |       |ssd1306, hd44780|
| bme280, aht20 |       | stepper, custom|       | nextion, auto |
+-------+-------+       +---------------+       +---------------+
        |                       |                       |
        +-----------------------+-----------------------+
                                |
                    +-----------v-----------+
                    |    ControlEngine      |
                    |  (PID, Bang-Bang,     |
                    |   Feedforward, Custom)|
                    +-----------+-----------+
                                |
                    +-----------v-----------+
                    |    SafetyEngine       |
                    | (Watchdog, Limits,    |
                    |  Thermal Runaway,     |
                    |  Sensor Validation)   |
                    +-----------------------+
```

### Driver Interface Contracts

All drivers implement standard interfaces:

**ISensorDriver**
```cpp
virtual bool begin(const JsonObject& config) = 0;
virtual SensorReading read() = 0;
virtual String getType() const = 0;
virtual String getName() const = 0;
virtual bool isConnected() = 0;
```

**IActuatorDriver**
```cpp
virtual bool begin(const JsonObject& config) = 0;
virtual bool setPower(float power_pct) = 0;
virtual void emergencyStop() = 0;
virtual ActuatorState getState() const = 0;
virtual String getType() const = 0;
```

**IDisplayDriver**
```cpp
virtual bool begin(const JsonObject& config) = 0;
virtual void clear() = 0;
virtual bool update(const JsonObject& status_fields) = 0;
virtual void showError(const String& message) = 0;
virtual void showBootScreen(const String& firmware_version) = 0;
virtual DisplayMetrics getMetrics() const = 0;
```

**IControlAlgorithm**
```cpp
virtual bool begin(const JsonObject& config) = 0;
virtual float compute(float target, float current, float dt) = 0;
virtual void reset() = 0;
virtual String getType() const = 0;
```

### Plugin Hook System (`IPlugin`)
Plugins register lifecycle callbacks to extend capabilities without modifying core firmware loops:
- `void onInit(ConfigManager& config)`
- `void onSessionStart(const DryingSession& session)`
- `void onTelemetryTick(const StatusPayload& telemetry)`
- `void onSessionStop(StopReason reason)`
- `void onFault(FaultCode fault, const char* message)`
- `bool handleWebSocketCommand(const String& topic, const JsonObject& payload, JsonObject& response)`
- `bool handleHttpRequest(const String& path, const JsonObject& params, String& response)`

---

## 3. Generic Hardware Support Matrix

### Sensor Drivers (Configurable via `type` string)
| Category | Driver Types | Communication | Examples |
|----------|--------------|---------------|----------|
| **Temp+Humidity** | `sht3x`, `sht30`, `aht20`, `bme280` | I2C | SHT31, SHT30, AHT10/20, BME280 |
| **Temp+Humidity** | `dht22`, `dht11`, `am2302` | 1-Wire (GPIO) | DHT22, AM2302 |
| **Temp Only** | `ds18b20`, `ds18s20` | 1-Wire | DS18B20, DS18S20 |
| **Temp Only** | `ntc`, `thermistor` | ADC | 10k/100k NTC with beta formula |
| **Temp+Pressure** | `bme280`, `bmp280` | I2C/SPI | BME280, BMP280 |
| **Custom** | `custom` | Any | User-provided plugin |

### Actuator Drivers (Configurable via `type` + `role`)
| Role | Driver Types | Control Methods | Examples |
|------|--------------|-----------------|----------|
| **Heater** | `mosfet_pwm`, `ssr`, `triac`, `custom` | PID, Bang-Bang, Feedforward | AOD4184, IRLB3034, SSR-25DA |
| **Fan** | `fan_pwm`, `fan_digital`, `shared_mosfet`, `custom` | PWM, ON/OFF, Shared | AOD4184, 12V/24V PWM fans |
| **Custom** | `stepper`, `servo`, `gpio`, `custom` | Position, Speed, PWM | 28BYJ-48, SG90, generic GPIO |

### Display Drivers (Configurable via `driver` + `bus`)
| Driver | Controller | Bus | Resolutions | Common Hardware |
|--------|------------|-----|-------------|-----------------|
| `ssd1306` | SSD1306 | I2C/SPI | 128x64, 128x32 | 0.96" OLED |
| `sh1106` | SH1106 | I2C/SPI | 128x64 | 1.3" OLED |
| `st7789` | ST7789 | SPI | 135x240, 240x240 | LilyGo T-Display, TTGO, T-QT |
| `ili9341` | ILI9341 | SPI | 240x320 | ESP32-2432S028 (CYD) |
| `st7735` | ST7735 | SPI | 128x128, 128x160 | 1.44"/1.8" Color TFT |
| `gc9a01` | GC9A01 | SPI | 240x240 | 1.28" Round Smartwatch TFT |
| `ili9488` | ILI9488 | SPI/8-bit | 320x480 | 3.5"/4.0" Color TFT |
| `hd44780` | HD44780 (PCF8574) | I2C | 16x2, 20x4 | Classic 1602/2004 LCD |
| `nextion` | Nextion | UART | Customizable | Nextion HMI |
| `auto` | Auto-detect | Any | Any | Best-effort |

### Control Algorithms (Configurable via `algorithm`)
| Algorithm | Parameters | Use Case |
|-----------|------------|----------|
| `pid` | `kp`, `ki`, `kd`, `max_integral` | Precision temperature control |
| `bang_bang` | `hysteresis_c`, `min_cycle_sec` | Simple on/off with hysteresis |
| `pwm_feedforward` | `base_pwm`, `temp_coefficient` | Known thermal characteristics |
| `custom` | User-defined | Plugin-provided algorithm |

---

## 4. Hardware Test Matrix (User Provided)

| Board | MCU | Display | Default I2C | Test Sensors | Test Actuators |
|-------|-----|---------|-------------|--------------|----------------|
| **ESP32_DEVKITC_V4** | ESP32-WROOM-32D | None/External | GPIO21/22 | SHT31 (I2C), DS18B20 (1-Wire) | AOD4184 MOSFET (PWM) |
| **LilyGo T-Display V1.1** | ESP32 + ST7789 135x240 SPI | ST7789 135x240 | GPIO21/22 | SHT31 (I2C) | AOD4184 MOSFET (PWM) |
| **ESP32-2432S028 (CYD)** | ESP32 + 2.8" 240x320 | ILI9341/ST7789 SPI | GPIO21/22 | SHT31 (I2C) | AOD4184 MOSFET (PWM) |

---

## 5. Fault Tolerance & Safety Matrix ("Tolerante a Falhas")

### Safe Abort Principle
Any anomaly during operation triggers `FAULT_STOPPED`. The heater actuator is **immediately forced LOW via hardware register write (<1ms)**. However, the ESP32 platform, WiFi, AsyncWebServer, WebSocket server, display, and logging subsystems **remain 100% operational**.

### Failure Conditions & Recovery Actions

| Failure Condition | Detection Criteria | Safety Action | Log Outcome |
|-------------------|--------------------|---------------|-------------|
| **Sensor Disconnect** | 3 consecutive failed reads (configurable timeout) | Heater PWM = 0%, Fan = 100% (cool down), State -> `FAULT_STOPPED` | Recorded in `drying.log` with sensor ID & pin |
| **Over-Temperature** | Chamber temp >= configurable limit (default 80°C) | Hard GPIO LOW to Heater, State -> `FAULT_STOPPED` | Critical fault logged with peak temp |
| **Thermal Runaway** | Heater >80% power for configurable time (45s) without configurable temp rise (0.5°C) | Heater PWM = 0%, State -> `FAULT_STOPPED` | Logged as `THERMAL_RUNAWAY` error |
| **I2C Bus Lockup** | SDA stuck LOW for >10ms | Execute I2C clock toggling recovery (9 cycles). If unrecovered -> `FAULT_STOPPED` | Bus recovery attempt & result logged |
| **Actuator Fault** | PWM output mismatch, overcurrent | Immediate cutoff, State -> `FAULT_STOPPED` | Actuator ID & fault logged |
| **NVS / Flash Corrupt** | Invalid checksum on config read | Load safe embedded factory defaults, activate "philarmony" AP | Warning logged in `system.log` |

---

## 6. Dual Logging Subsystem Specification

### Architecture
1. **`system.log` (`/littlefs/system.log`)**:
   - Initialized/rotated on boot.
   - Logs boot details, MAC address, assigned IP, WiFi RSSI, sensor initialization, NVS config status, system warnings.
   - Available via HTTP `GET /log/system`.

2. **`drying.log` (`/littlefs/drying.log`)**:
   - Recreated (truncated) at the **START** of a new drying cycle (`control/start`).
   - Records: cycle parameters, target temp/humidity, 10s telemetry snapshots, heater duty cycle, fan state, manual stop events, and exact fault reasons if aborted.
   - **Crucial Requirement**: NOT deleted or cleared when the cycle finishes or fails. Preserved post-mortem across reboots for inspection.
   - Available via HTTP `GET /log/drying`.

3. **WebSocket Log Streaming**:
   - Topic `logs/stream` broadcasts formatted log lines in real-time to connected web clients.

---

## 7. PID Auto-Calibration Routine (PID_CALIBRATE)

### Algorithm & Method
Inspired by Klipper's `PID_CALIBRATE` command, the firmware implements the **Ziegler-Nichols Relay Auto-Tuning** method:
- Cycles the heater PWM output between 0% and 100% (or configured `heater_max_power_pct`) around a target calibration temperature (e.g. 50°C).
- Measures the temperature oscillation period ($T_u$) and peak-to-peak amplitude ($a$).
- Calculates ultimate gain $K_u = \frac{4d}{\pi \cdot a}$ (where $d$ is PWM drive step).
- Derives classical PID coefficients:
  - $K_p = 0.6 \cdot K_u$
  - $K_i = \frac{2 \cdot K_p}{T_u}$
  - $K_d = \frac{K_p \cdot T_u}{8}$
- Automatically stores $K_p, K_i, K_d$ into NVS memory under `heater.pid` namespace for seamless boot persistence.

---

## 8. Developer Tooling — Root Makefile (DEC-009)

### Decision
Add a **GNU Make** façade at repository root that wraps PlatformIO CLI for the primary workflows: **compile**, **native tests**, and **ESP32 flash/install**. PlatformIO remains the source of truth for environments, libraries, and partitions.

### Rationale
- Short, memorable targets (`make build`, `make test`, `make flash`) lower contributor friction vs. repeating `pio run -e …`.
- Exit codes propagate from `pio` so CI and constitution “tests must fail the build” stay intact.
- `ENV=` selects among existing `platformio.ini` envs without duplicating board config.
- Aligns with Constitution **Workspace de Dependências** (document Make + PlatformIO in `.vscode/workspace.json`) and **Documentação Sincronizada** (quickstart/README prefer `make`).

### Alternatives considered
| Option | Why rejected |
|--------|----------------|
| Shell scripts only (`scripts/build.sh`) | Fragmented; no standard `make help` discovery |
| CMake as primary build | Conflicts with PlatformIO’s managed toolchain; high migration cost |
| Taskfile / just | Extra runtime dependency; Make is ubiquitous on macOS/Linux CI |
| Document `pio` only | User explicitly requested Makefile for principal tasks |

### Target contract
See `contracts/makefile-targets.md`. No firmware runtime or data-model impact.