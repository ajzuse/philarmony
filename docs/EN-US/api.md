# Philarmony — API Reference (EN-US)

## Overview

ESP32 filament-dryer firmware. Real-time control over WebSocket `ws://<ip>/ws` (HTTP port 80). First-boot WiFi setup via AP `philarmony` / `philarmony` at `http://192.168.4.1/`.

Envelope: `{"topic":"<name>","payload":{...}}`. Errors use `topic` `<name>/error` with payload key `error`.

## WebSocket commands

### `control/start`
```json
{"topic":"control/start","payload":{"profile_id":"pla","target_temp_c":50,"max_duration_min":240,"target_humidity_pct":15}}
```
Ack: `{"topic":"control/start","payload":{"status":"started","profile_id":"pla"}}`

### `control/stop`
Allowed in `drying` or `cooldown`. Cuts heater/fan and ends cooldown.
```json
{"topic":"control/stop","payload":{"reason":"user_requested"}}
```

### `status/subscribe` / `status/unsubscribe`
Subscribe to 1 Hz `status/update`.

### `config/hardware`
Klipper-style `sensors[]`, `actuators[]`, nested `display`, `control`. Rejected while drying/cooldown.
Ack topic: `config/hardware/response` with `{"status":"saved"}`.

### `config/display` / `config/control`
Display layout (`fields`, `font_scaling`, `compact_mode`, `refresh_rate_hz`) and control algorithm + `safety_limits`.

### `config/profiles/*`
`list` | `get` | `create` | `update` | `delete` | `reset_defaults`.  
Get wraps `{profile:{...}}`. Create/update ack: `{status:"created"|"updated", profile_id}`. Custom profiles include `created_at` / `updated_at`. Custom ID shadows builtin.

### `control/pid_calibrate`
```json
{"topic":"control/pid_calibrate","payload":{"target_temp_c":50,"cycles":5}}
```
Progress/complete on `status/pid_calibrate` (`status:"calibrating"|"complete"`, `saved_to_nvs` only after NVS write).

## Telemetry — `status/update`

Lowercase `status`: `ready`, `drying`, `cooldown`, `stopped`, `fault_stopped`, …  
Fields include `chamber_temp_c`, `humidity_pct`, heater/fan power and booleans, `cpu_usage_pct`, `memory_free_bytes`, `uptime_sec`, `session_id`, `stop_reason`, elapsed/remaining.

## Faults — `status/fault`

`fault_code` string enum, e.g. `SENSOR_DISCONNECT`, `OVER_TEMPERATURE`, `THERMAL_RUNAWAY`, `SENSOR_RATE_OF_CHANGE`, `ACTUATOR_FAULT`, `SPI_BUS_ERROR`.

## Logs — `logs/stream`

`target_log`: `"drying"` | `"system"` (bool `drying` kept as alias).

## HTTP

| Method | Path | Notes |
|--------|------|-------|
| GET | `/` | Captive WiFi form (AP) |
| POST | `/api/wifi/config` | Form or JSON `{ssid,password}` → `{status,message}` |
| GET | `/api/info` | `firmware_version`, `chip_model`, `mac_address`, `free_heap_bytes`, `system_status`, `active_feature` |
| GET/POST | `/api/hardware/config` | Same Klipper schema as WS `config/hardware` |
| GET | `/log/system`, `/log/drying` | `text/plain; charset=utf-8` + `Content-Disposition: attachment` |

Captive probes (`/generate_204`, `/fwlink`, `/hotspot-detect.html`, `/ncsi.txt`, `/connecttest.txt`) **redirect** to `/`.

## Full contracts

See `specs/001-filament-dryer-esp32/contracts/websocket-api.md` and `http-api.md`.

## Architecture

See [architecture.md](architecture.md) and [configuration.md](configuration.md).
