# Flash Pipeline Contract

## Stages (ordered)

| Stage | Event `flash_stage` | Success criteria |
|-------|---------------------|------------------|
| 1. Preflight | `preflight` | Device present; profile valid (incl. WiFi password present in-session); artifact + esptool checksum OK; PinValidator OK |
| 2. Erase | `erasing` | esptool erase completed |
| 3. Write app | `writing_app` | Application image written |
| 4. Write NVS/config | `writing_nvs` | Config partition/image written |
| 5. Verify | `verifying` | esptool verify/readback OK → **Flash Success gate** |
| 6. Reset | `resetting` | Chip reset issued |
| 7. Done | `success` | Job complete (flash succeeded) |
| 8. Network check (optional) | `network_verify` | Soft only — see below |

Failure at stages 1–6 → `failed` with `error` + log buffer (FR-011).

### Optional post-flash network check

After `success` (or as a non-fatal follow-up after reset):

- Attempt WS/HTTP **only if** target reachable: hotspot `philarmony` / `192.168.4.1`, or user-supplied IP.
- Outcomes: `network_ok` | `network_skipped` | `network_warn` (unreachable / timeout).
- `network_warn` / `network_skipped` MUST **not** change Flash Success to failed.

## Failure / Retry (no automatic restore)

- There is **no** pre-erase firmware dump or automatic rollback restore.
- UI **Retry** re-runs stages 1–7 with the **same current** bundled `FirmwarePackage` + in-session profile.
- USB unplug mid-flash → `failed`; user reconnects and Retries; warn partial state possible.

## Flutter orchestration

| API | Role |
|-----|------|
| `DeviceDetector.listPorts()` | `flutter_libserialport` enumeration + VID/PID hints |
| `PinValidator.validate(profile)` | Shared `philarmony_core` — blocks flash |
| `FirmwareFlasher.start(port, profile)` | Spawns bundled esptool; streams stdout → progress |
| `PostFlashVerifier.tryReach(host)` | Optional soft WS/HTTP; never overrides esptool success |
| `ProfileStore.export/import` | JSON I/O; password always omit/`***` on export |
| `LastProfileStore` | Local last-session DeviceProfile sans password (reconfigure) |

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
- Reading on-device NVS over USB for reconfigure
- Automatic restore of a dumped pre-erase image
