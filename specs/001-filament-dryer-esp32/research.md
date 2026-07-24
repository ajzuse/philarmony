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

---

## 2. Klipper-Inspired Object Architecture & Plugin System

### Architecture
Inspired by Klipper 3D printer firmware, the system represents hardware and software features as **Configurable Named Objects**. A central `ObjectRegistry` parses hardware definitions from NVS/JSON configuration at boot and instantiates drivers dynamically.

```text
               +-----------------------+
               |    ObjectRegistry     |
               +-----------+-----------+
                           |
       +-------------------+-------------------+
       |                   |                   |
+------v------+     +------v------+     +------v------+
| ISensorDriver|     |IActuatorDrv |     |IDisplayDrv  |
+------+------+     +------+------+     +------+------+
       |                   |                   |
   +---+---+           +---+---+           +---+---+
   |SHT31  |           |AOD4184|           |LovyanGFX|
   |DS18B20|           |  PWM  |           | (Multi) |
   +-------+           +-------+           +-------+
```

### Plugin Hook System (`IPlugin`)
Plugins register lifecycle callbacks to extend capabilities without modifying core firmware loops:
- `void onInit(ConfigManager& config)`
- `void onSessionStart(const DryingSession& session)`
- `void onTelemetryTick(const StatusPayload& telemetry)`
- `void onSessionStop(StopReason reason)`
- `void onFault(FaultCode fault, const char* message)`

---

## 3. Expanded Display Drivers Architecture & Ecosystem Matrix

To ensure maximum hardware compatibility across the ESP32 maker ecosystem, display rendering uses **LovyanGFX / Adafruit GFX / U8g2 abstraction wrappers**, providing zero-copy SPI DMA memory transfers and hardware-accelerated drawing.

### Supported Display Controllers & Bus Types

| Driver Family | Controller IC | Common Bus Types | Resolutions Supported | Popular Hardware Modules / Examples |
|---------------|---------------|------------------|-----------------------|-------------------------------------|
| **OLED Monochrome** | `SSD1306` | I2C / SPI | 128x64, 128x32 | Standard 0.96" / 0.91" I2C OLEDs |
| **OLED Monochrome** | `SH1106` | I2C / SPI | 128x64 | 1.3" I2C OLED displays |
| **OLED Monochrome** | `SSD1309` | I2C / SPI | 128x64 | 2.42" large OLED displays |
| **TFT Small Color** | `ST7789` | SPI | 135x240, 240x240, 170x320 | LilyGo T-Display V1.1, TTGO, T-QT |
| **TFT Medium Color**| `ILI9341` | SPI / Parallel-8 | 240x320 | ESP32-2432S028 (CYD), Red SPI TFTs |
| **TFT Compact Color**| `ST7735` | SPI | 128x128, 128x160 | 1.44" / 1.8" Color TFT modules |
| **Round Color TFT** | `GC9A01` | SPI | 240x240 | 1.28" Round Smartwatch Style TFTs |
| **TFT Large Color** | `ILI9488` / `ST7796` | SPI / Parallel-8 | 320x480 | 3.5" / 4.0" Color TFT displays |
| **Character LCD**  | `HD44780` (PCF8574)| I2C | 16x2, 20x4 | Classic 1602 / 2004 LCDs with I2C backpack |
| **Smart HMI**      | `Nextion` | UART Serial | Customizable | Nextion Basic/Enhanced/Intelligent HMI |

### Display Layout Engine
- **Layout Auto-Adaptation**: Automatically adjusts font size, icon positioning, and field density according to detected display width and height.
- **Configurable Display Refresh Rate**: Refresh cycle decoupled from status stream (e.g. 1Hz - 5Hz configurable, default 1Hz).

---

## 4. Hardware Support Matrix & Test Hardware Mapping

### Test Board Configurations
1. **ESP32_DEVKITC_V4**:
   - ESP32-WROOM-32D (4MB Flash). Standard development board.
   - Default I2C: SDA (GPIO21), SCL (GPIO22).
   - PWM Heater: GPIO25 (MOSFET AOD4184).
   - PWM Fan: GPIO26 (MOSFET AOD4184).

2. **LilyGo T-Display V1.1**:
   - ESP32 + ST7789 1.14" IPS Display (135x240 resolution, SPI).
   - Display Pins: MOSI (GPIO19), SCLK (GPIO18), CS (GPIO5), DC (GPIO16), RST (GPIO23), BL (GPIO4).
   - I2C Sensors: SDA (GPIO21), SCL (GPIO22).

3. **ESP32-2432S028 (Cheap Yellow Display - CYD)**:
   - ESP32 + 2.8" TFT 240x320 Display (ILI9341/ST7789, SPI).
   - Display Pins: MOSI (GPIO13), MISO (GPIO12), SCLK (GPIO14), CS (GPIO15), DC (GPIO2), RST (NC/3.3V), BL (GPIO21).

### Sensor Drivers
- **Integrated Temp + Humidity**: SHT31 (I2C `0x44`/`0x45`), SHT30, DHT22 (1-Wire digital), BME280 (I2C `0x76`/`0x77`), AHT10/AHT20.
- **Dedicated Temp-Only**: DS18B20 (1-Wire), NTC Thermistor (ADC with Beta formula), PT100/MAX31865.
- **Combined/Separate Sensor Resolution**: The `SensorManager` can bind a single integrated sensor (e.g. SHT31 for both temp and humidity) OR two independent drivers (e.g., DS18B20 for temperature + DHT22 for humidity).

### Actuator Configurations (MOSFET AOD4184)
- **Shared MOSFET (Single Output)**: One MOSFET controls both heater element and fan tied to the same power output.
- **Independent Dual MOSFET**:
  - Heater: MOSFET AOD4184 on GPIO X with LEDC PWM frequency 100Hz - 1kHz.
  - Exhaust Fan: MOSFET AOD4184 on GPIO Y with LEDC PWM or Digital GPIO HIGH/LOW.

---

## 5. Fault Tolerance & Safety Matrix ("Tolerante a Falhas")

### Safe Abort Principle
Any anomaly during operation triggers `FAULT_STOPPED`. The heater MOSFET is **immediately forced LOW via hardware register write (<1ms)**. However, the ESP32 platform, WiFi, AsyncWebServer, WebSocket server, display, and logging subsystems **remain 100% operational**.

### Failure Conditions & Recovery Actions

| Failure Condition | Detection Criteria | Safety Action | Log Outcome |
|-------------------|--------------------|---------------|-------------|
| **Sensor Disconnect** | 3 consecutive failed reads (600ms timeout) | Heater PWM = 0%, Fan = 100% (cool down), State -> `FAULT_STOPPED` | Recorded in `drying.log` with sensor ID & pin |
| **Over-Temperature** | Chamber temp >= 80.0°C | Hard GPIO LOW to Heater MOSFET, State -> `FAULT_STOPPED` | Critical fault logged with peak temp |
| **Thermal Runaway** | Heater >80% power for 45s without +0.5°C temp rise | Heater PWM = 0%, State -> `FAULT_STOPPED` | Logged as `THERMAL_RUNAWAY` error |
| **I2C Bus Lockup** | SDA stuck LOW for >10ms | Execute I2C clock toggling recovery sequence (9 clock cycles). If unrecovered -> `FAULT_STOPPED` | Bus recovery attempt & result logged |
| **NVS / Flash Corrupt**| Invalid checksum on config read | Load safe embedded factory defaults, activate "philarmony" AP | Warning logged in `system.log` |

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

