# Flash Pipeline Contract

## Stages (ordered)

| Stage | Event `flash_stage` | Success criteria |
|-------|---------------------|------------------|
| 1. Preflight | `preflight` | Device present; profile valid; artifact + esptool checksum OK |
| 2. Erase | `erasing` | esptool erase completed |
| 3. Write app | `writing_app` | Application image written |
| 4. Write NVS/config | `writing_nvs` | Config partition/image written |
| 5. Verify | `verifying` | Verify/readback OK |
| 6. Reset | `resetting` | Chip reset issued |
| 7. Done | `success` | Job complete |

Failure → `failed` with `error` + log buffer (FR-011).

## Flutter orchestration

| API | Role |
|-----|------|
| `DeviceDetector.listPorts()` | `flutter_libserialport` enumeration + VID/PID hints |
| `PinValidator.validate(profile)` | Shared `philarmony_core` — blocks flash |
| `FirmwareFlasher.start(port, profile)` | Spawns bundled esptool with args; streams stdout → progress |
| `ProfileStore.export/import` | JSON I/O; strip password on export |

## Progress stream

```json
{
  "job_id": "…",
  "stage": "writing_app",
  "progress_pct": 42,
  "message": "Writing at 0x10000"
}
```

Parsed from esptool stdout/stderr; UI listens on a Dart `Stream`.

## Non-goals

- Compiling firmware from source
- Simultaneous multi-device flash
- OTA
- Reimplementing Espressif ROM protocol in pure Dart (MVP)
