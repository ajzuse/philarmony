# Contract: Pause / Resume API

**Feature**: `004-esp32-touchscreen-ui`  
**Extends**: `specs/001-filament-dryer-esp32/contracts/websocket-api.md`  
**Note**: Control app `003` remains Stop-only until it opts into these topics (DEC-012).

## control/pause

**Direction**: Client → ESP32 (also invoked internally by touch)

```json
{
  "topic": "control/pause",
  "payload": {
    "reason": "user_requested"
  }
}
```

**Preconditions**: `status == "drying"`  
**Effects**:
- Transition `DRYING` → `PAUSED`
- Heater power 0% within 500ms
- Fan off (MVP)
- Freeze elapsed timer accrual
- Ack + `status/update` with `status: "paused"`, `paused: true`

**Errors**: `invalid_state`, `fault_active`

## control/resume

```json
{
  "topic": "control/resume",
  "payload": {}
}
```

**Preconditions**: `status == "paused"`; SafetyEngine healthy  
**Effects**:
- Transition `PAUSED` → `DRYING`
- Restore control loop with existing session targets
- Resume elapsed timer
- `status: "drying"`, `paused: false`

**Errors**: `invalid_state`, `safety_block`

## status/update fields

| Field | When paused |
|-------|-------------|
| `status` | `"paused"` |
| `paused` | `true` |
| `heater_on` | `false` |
| `heater_power_pct` | `0` |
| `elapsed_sec` | frozen value |
| `ui_source` | `touch` \| `websocket` \| `auto` |

## Stop while paused

`control/stop` (and touch Stop) MUST be accepted from `PAUSED`, same safety cutoff as from `DRYING`.
