# Contract: WebSocket Client (Control App)

**Feature**: `003-filament-dryer-control-app`  
**Canonical firmware API**: `specs/001-filament-dryer-esp32/contracts/websocket-api.md`  
**Date**: 2026-08-10

## Transport

| Item | Value |
|------|-------|
| Scheme | `ws://` (TLS deferred) |
| Default URL | `ws://{host}/ws` |
| Default port | `80` |
| Frame | JSON object `{ "topic": string, "payload": object }` |

Manual override: user may set host + port + path. Discovery may supply host/port from mDNS TXT/SRV.

## Client responsibilities

1. Open socket → send `status/subscribe`.
2. Parse inbound topics; ignore unknown topics (forward-compatible).
3. Correlate request/response topics (`config/*/response`, `config/*/error`).
4. Reconnect with exponential backoff (cap 60s); re-subscribe after reconnect.
5. Queue outbound commands while disconnected; flush in order on reconnect (except obsolete start if stop queued later — last control intent wins).
6. Never send config/control that fails local validation.

## Topics used (MVP)

### Outbound

| Topic | Purpose |
|-------|---------|
| `status/subscribe` / `status/unsubscribe` | Telemetry |
| `control/start` | Start cycle (`profile_id` and/or explicit params; field `max_duration_min`) |
| `control/stop` | Stop (`reason`) |
| `config/hardware` | Sensors/actuators/display/control blob per 001 |
| `config/display` | Display-only updates when applicable |
| `config/control` | PID/control subsection when applicable |
| `config/profiles/list\|get\|create\|update\|delete\|reset_defaults` | Material profiles |

### Inbound

| Topic | Purpose |
|-------|---------|
| `status/update` | 1Hz telemetry |
| `status/fault` | Safety / fault banner |
| `logs/stream` | Optional debug pane |
| `status/pid_calibrate` | Optional advanced UI |
| `config/profiles/*/response` | Profile sync |
| `config/hardware/response` / `config/hardware/error` | Config ack |

## Envelope schema (normative for client codec)

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "philarmony-ws-envelope",
  "type": "object",
  "required": ["topic", "payload"],
  "properties": {
    "topic": { "type": "string", "minLength": 1 },
    "payload": { "type": "object" }
  },
  "additionalProperties": false
}
```

Payload field shapes MUST match 001 contract examples (do not invent parallel field names).

## Non-goals

- Authentication headers / TLS (v1)
- Multiplex protocols other than JSON topic framing
- Emulating installer USB/flash over this socket
- `control/pause` / `control/resume` (out of MVP until firmware documents them)
