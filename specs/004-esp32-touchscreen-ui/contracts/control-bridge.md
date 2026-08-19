# Contract: Touch ↔ Firmware Control Bridge

**Feature**: `004-esp32-touchscreen-ui`  
**Normative peer**: `specs/001-filament-dryer-esp32/contracts/websocket-api.md`

## Principle

Touch UI and WebSocket clients share one control authority: `ProfileManager` + `StateMachine` + `SafetyEngine`. UI never writes PWM directly.

## Internal operations

| UI action | Internal API | WS equivalent |
|-----------|--------------|---------------|
| Confirm preset start | `buildSessionFromRequest({profile_id})` + `startDrying` | `control/start` |
| Confirm custom start | `buildSessionFromRequest(explicit)` + `startDrying` | `control/start` |
| Pause | `pauseDrying()` | `control/pause` |
| Resume | `resumeDrying()` | `control/resume` |
| Stop (confirmed) | `stopDrying(USER_STOPPED)` / cooldown path | `control/stop` |

## Broadcast

After every successful touch command:
1. Update UI screens from `StateMachine`
2. Broadcast `status/update` to WS subscribers with `ui_source: "touch"`
3. On failure: toast + keep prior screen; no partial actuator state

## Conflict policy

- Concurrent WS + touch: **last successful command wins**
- Visual feedback on device when WS changes state mid-interaction
- Safety/fault always wins over user pause/start

## Validation

Reuse ProfileManager range checks; reject with UI message mapping existing error strings (PT/EN).
