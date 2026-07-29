# Philarmony — Arquitetura (PT-BR)

## Layout de runtime

- **Core 1**: loop 50 Hz — sensores, SafetyEngine, ControlEngine / autotune PID, PWM, telemetria 1 Hz.
- **Core 0**: WiFi, HTTP/WebSocket async, tarefa de display, SystemMetrics.

## Módulos

| Módulo | Função |
|--------|--------|
| `ConfigManager` | NVS WiFi, sensores, atuadores, display, perfis, PID |
| `StateMachine` | BOOT → WIFI/HOTSPOT → READY → DRYING → COOLDOWN → STOPPED / FAULT_STOPPED |
| `SafetyEngine` | Limite térmico, timeout de sensor, runaway, recuperação I2C/SPI, falha de atuador, WDT |
| `DriverRegistry` | Factories de sensores, atuadores, displays |
| `HardwareConfigParser` | Validação JSON estilo Klipper |
| `ControlEngine` | PID / bang-bang / feedforward |
| `WifiManager` | STA não bloqueante + AP `philarmony` |
| `WebServer` / `WebSocketServer` | Logs/config HTTP + API tempo real |
| `DisplayManager` | Auto-detect + layout |
| `PluginManager` | Hooks para tópicos/rotas desconhecidos |
| `LogManager` | LittleFS `system.log` / `drying.log` |

## Modelo de segurança

Em falha: corte imediato dos atuadores; rede e logs permanecem (`FAULT_STOPPED`). Hot-reload de hardware é rejeitado em DRYING/COOLDOWN. Desconexão de sensor aguarda `sensor_timeout_ms`; heater em 0% enquanto a leitura for inválida.
