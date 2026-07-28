# Philarmony — Architecture (EN-US)

Modular C++/PlatformIO firmware (Klipper-inspired):

- `core/` — ConfigManager, StateMachine, SafetyEngine, ProfileManager, DriverRegistry
- `drivers/` — pluggable sensors, actuators, displays
- `control/` — PID, bang-bang, feedforward
- `network/` — WifiManager (non-blocking STA + AP + DNS captive), WebServer, WebSocketServer
- `plugins/` — extension hooks

FreeRTOS dual-core: Core 1 control loop (~50 Hz); Core 0 network/display.
