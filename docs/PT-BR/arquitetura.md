# Philarmony — Arquitetura (PT-BR)

Firmware modular C++/PlatformIO inspirado em Klipper:

- `core/` — ConfigManager, StateMachine, SafetyEngine, ProfileManager, DriverRegistry
- `drivers/` — sensores, atuadores e displays plugáveis
- `control/` — PID, bang-bang, feedforward
- `network/` — WifiManager (STA não-bloqueante + AP + DNS captive), WebServer, WebSocketServer
- `plugins/` — ganchos de extensão

Dual-core FreeRTOS: Core 1 loop de controle (~50 Hz); Core 0 rede/display.
