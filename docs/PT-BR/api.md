# Philarmony — Referência de API (PT-BR)

## Visão geral

Firmware ESP32 da secadora. Controle em tempo real via WebSocket `ws://<ip>/ws` (porta HTTP 80). Configuração WiFi inicial no AP `philarmony` / `philarmony` em `http://192.168.4.1/`.

Envelope: `{"topic":"<nome>","payload":{...}}`. Erros: tópico `<nome>/error` com chave `error` no payload.

## Comandos WebSocket

### `control/start`
```json
{"topic":"control/start","payload":{"profile_id":"pla","target_temp_c":50,"max_duration_min":240,"target_humidity_pct":15}}
```
Ack: `{"topic":"control/start","payload":{"status":"started","profile_id":"pla"}}`

### `control/stop`
Permitido em `drying` ou `cooldown`. Corta aquecedor/ventilador e cancela cooldown.
```json
{"topic":"control/stop","payload":{"reason":"user_requested"}}
```

### `status/subscribe` / `status/unsubscribe`
Assina telemetria 1 Hz (`status/update`).

### `config/hardware`
Schema Klipper: `sensors[]`, `actuators[]`, `display`, `control`. Rejeitado durante secagem/cooldown.
Ack: tópico `config/hardware/response` com `{"status":"saved"}`.

### `config/display` / `config/control`
Layout (`fields`, `font_scaling`, `compact_mode`, `refresh_rate_hz`) e algoritmo + `safety_limits`.

### `config/profiles/*`
`list` | `get` | `create` | `update` | `delete` | `reset_defaults`.  
Get: `{profile:{...}}`. Create/update: `{status:"created"|"updated", profile_id}`. Perfis custom incluem `created_at`/`updated_at`. ID custom sobrescreve builtin.

### `control/pid_calibrate`
```json
{"topic":"control/pid_calibrate","payload":{"target_temp_c":50,"cycles":5}}
```
Progresso em `status/pid_calibrate` (`calibrating`/`complete`; `saved_to_nvs` só após gravação NVS).

## Telemetria — `status/update`

`status` em minúsculas: `ready`, `drying`, `cooldown`, `stopped`, `fault_stopped`, …  
Campos: temperaturas, umidade, potência heater/fan, `cpu_usage_pct`, `memory_free_bytes`, `uptime_sec`, `session_id`, `stop_reason`, elapsed/remaining.

## Falhas — `status/fault`

`fault_code` string: `SENSOR_DISCONNECT`, `OVER_TEMPERATURE`, `THERMAL_RUNAWAY`, `SENSOR_RATE_OF_CHANGE`, `ACTUATOR_FAULT`, `SPI_BUS_ERROR`, etc.

## Logs — `logs/stream`

`target_log`: `"drying"` | `"system"` (bool `drying` como alias).

## HTTP

| Método | Caminho | Notas |
|--------|---------|-------|
| GET | `/` | Formulário WiFi (AP) |
| POST | `/api/wifi/config` | Form ou JSON → `{status,message}` |
| GET | `/api/info` | Inclui `active_feature` |
| GET/POST | `/api/hardware/config` | Mesmo schema do WS |
| GET | `/log/system`, `/log/drying` | `text/plain; charset=utf-8` + attachment |

Probes de captive portal redirecionam para `/`.

## Contratos completos

`specs/001-filament-dryer-esp32/contracts/websocket-api.md` e `http-api.md`.

## Arquitetura

Ver [arquitetura.md](arquitetura.md) e [configuracao.md](configuracao.md).
