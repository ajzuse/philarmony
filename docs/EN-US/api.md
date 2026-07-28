# Philarmony — API Reference (EN-US)

## Overview

ESP32 firmware for a DIY filament dryer. Real-time control/telemetry over WebSocket at `ws://<ip>/ws`. Initial WiFi setup via hotspot `philarmony` / `philarmony` at `http://192.168.4.1/`.

## Core WebSocket topics

| Topic | Direction | Description |
|-------|-----------|-------------|
| `control/start` | client→ESP | Start cycle (profile or explicit params) |
| `control/stop` | client→ESP | Stop heater/ventilation |
| `status/subscribe` | client→ESP | Subscribe to 1 Hz telemetry |
| `status/update` | ESP→client | Live status payload |
| `status/fault` | ESP→client | Fault with `fault_code` string |
| `config/hardware` | client→ESP | Sensors, actuators, display, control |
| `config/display` | client→ESP | Layout and refresh rate |
| `config/profiles/*` | client→ESP | Filament profile CRUD |
| `control/pid_calibrate` | client→ESP | PID auto-tune (`cycles` or `max_cycles`) |

## Safety

Configurable hard temperature limit (default 80 °C), watchdog, actuator emergency cutoff, and HTTP logs at `/log/system` and `/log/drying`.

## Architecture

See [architecture.md](architecture.md).
