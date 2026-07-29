# Philarmony — Architecture (EN-US)

## Runtime layout

- **Core 1**: 50 Hz control loop — sensors, SafetyEngine, ControlEngine / PID autotune, actuator PWM, 1 Hz telemetry.
- **Core 0**: WiFi, Async HTTP + WebSocket, display refresh task, SystemMetrics.

## Modules

| Module | Role |
|--------|------|
| `ConfigManager` | NVS WiFi, sensors, actuators, display, profiles, PID |
| `StateMachine` | BOOT → WIFI/HOTSPOT → READY → DRYING → COOLDOWN → STOPPED / FAULT_STOPPED |
| `SafetyEngine` | Hard temp limit, sensor timeout, thermal runaway, I2C/SPI recovery, actuator fault, WDT |
| `DriverRegistry` | Factory for sensors, actuators, displays |
| `HardwareConfigParser` | Validate Klipper-style JSON (GPIO ranges, pin conflicts, type allow-lists) |
| `ControlEngine` | Switchable PID / bang-bang / PWM feedforward |
| `WifiManager` | Non-blocking STA + AP `philarmony` fail-fast |
| `WebServer` / `WebSocketServer` | HTTP logs/config + real-time API |
| `DisplayManager` | Auto-detect + layout (font_scaling, compact_mode) |
| `PluginManager` | Extension hooks for unknown WS/HTTP paths |
| `LogManager` | LittleFS `system.log` / `drying.log` |

## Safety model

On fault: actuators cut immediately; network and logs stay up (`FAULT_STOPPED`). Hardware hot-reload is rejected during DRYING/COOLDOWN. Sensor disconnect waits `sensor_timeout_ms` before abort; heater power limited to 0% while readings are invalid.

## Data flow

Sensors → calibration → SafetyEngine → ControlEngine → actuators. Status mirrored to WebSocket subscribers and DisplayManager at independent rates.
