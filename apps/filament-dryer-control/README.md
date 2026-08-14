# Philarmony Filament Dryer Control

Cross-platform Flutter app (Android, iOS, Windows, macOS, Linux) for monitoring and controlling Philarmony filament dryers over the LAN.

## Paths

| Item | Path |
|------|------|
| App | `apps/filament-dryer-control/` |
| Shared core | `packages/philarmony_core/` |
| Spec | `specs/003-filament-dryer-control-app/` |
| Manual stages | `specs/005-manual-validation/` (**VS-CTRL-1…9**) |
| Packaging notes | `apps/filament-dryer-control/packaging/` |

## WebSocket

- Default: `ws://{host}:80/ws` per firmware `001` contract
- Topics: `status/subscribe`, `control/start`, `control/stop`, `config/hardware`, `config/profiles/*`, …

## Development

```bash
make run-control          # fake WS by default
make test-control
cd apps/filament-dryer-control && fvm flutter run --dart-define=USE_FAKE_WS=false
```

## Packaging

```bash
make package-control-linux
make package-control-macos    # macOS host
make package-control-windows  # Windows host
```

## Mobile background & store notes

### Android

- Permissions: `FOREGROUND_SERVICE`, `FOREGROUND_SERVICE_DATA_SYNC`, `POST_NOTIFICATIONS`, multicast/Wi‑Fi
- Service: `CycleForegroundService` with `foregroundServiceType="dataSync"`
- Justification for Play: keep WebSocket alive **only while a drying cycle is active** (user-initiated); stop when cycle ends
- Dart lifecycle: `AndroidBackgroundSession` started/stopped from session status

### iOS

- `NSLocalNetworkUsageDescription` + Bonjour `_philarmony._tcp`
- `UIBackgroundModes`: `fetch`, `processing`
- `BGTaskSchedulerPermittedIdentifiers`: `com.philarmony.filament_dryer_control.cycle`
- App Store note: background networking is limited to active user-started cycles; no silent continuous tracking

## License

GPL-3.0-or-later
