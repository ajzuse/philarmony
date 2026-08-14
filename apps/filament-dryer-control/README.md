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
- Dart lifecycle: `AndroidBackgroundSession` starts/stops the FGS via `philarmony/background` MethodChannel; sets `backgroundAllowed=false` when OS denies

### iOS

- `NSLocalNetworkUsageDescription` + Bonjour `_philarmony._tcp`
- `UIBackgroundModes`: `fetch`, `processing`
- `BGTaskSchedulerPermittedIdentifiers`: `com.philarmony.filament_dryer_control.cycle`
- App Store note: background networking is limited to active user-started cycles; no silent continuous tracking
- Dart lifecycle: `IosBackgroundSession` uses the same MethodChannel (stub returns success; relies on background modes)

## License

GPL-3.0-or-later

---

# Philarmony Controle de Secador de Filamento

Aplicativo Flutter multiplataforma (Android, iOS, Windows, macOS, Linux) para monitorar e controlar secadores Philarmony na rede local.

## Caminhos

| Item | Caminho |
|------|---------|
| App | `apps/filament-dryer-control/` |
| Core compartilhado | `packages/philarmony_core/` |
| Especificação | `specs/003-filament-dryer-control-app/` |
| Validação manual | `specs/005-manual-validation/` (**VS-CTRL-1…9**) |
| Empacotamento | `apps/filament-dryer-control/packaging/` |

## WebSocket

- Padrão: `ws://{host}:80/ws` conforme contrato de firmware `001`
- Tópicos: `status/subscribe`, `control/start`, `control/stop`, `config/hardware`, `config/profiles/*`, …

## Desenvolvimento

```bash
make run-control          # WS simulado por padrão
make test-control
cd apps/filament-dryer-control && fvm flutter run --dart-define=USE_FAKE_WS=false
```

## Empacotamento

```bash
make package-control-linux
make package-control-macos    # host macOS
make package-control-windows  # host Windows
```

## Segundo plano (mobile) e notas para lojas

### Android

- Permissões: `FOREGROUND_SERVICE`, `FOREGROUND_SERVICE_DATA_SYNC`, `POST_NOTIFICATIONS`, multicast/Wi‑Fi
- Serviço: `CycleForegroundService` com `foregroundServiceType="dataSync"`
- Justificativa na Play Store: manter o WebSocket ativo **somente durante um ciclo de secagem ativo** (iniciado pelo usuário); encerra ao fim do ciclo
- Ciclo de vida Dart: `AndroidBackgroundSession` inicia/para o FGS via MethodChannel `philarmony/background`; define `backgroundAllowed=false` quando o SO nega

### iOS

- `NSLocalNetworkUsageDescription` + Bonjour `_philarmony._tcp`
- `UIBackgroundModes`: `fetch`, `processing`
- `BGTaskSchedulerPermittedIdentifiers`: `com.philarmony.filament_dryer_control.cycle`
- Nota App Store: rede em segundo plano limitada a ciclos iniciados pelo usuário; sem rastreamento contínuo silencioso
- Ciclo de vida Dart: `IosBackgroundSession` usa o mesmo MethodChannel (stub retorna sucesso; depende dos modos de segundo plano)

## Licença

GPL-3.0-or-later
