# Contract: UI Screens

**Feature**: `004-esp32-touchscreen-ui`

## Navigation graph (P1)

```text
[Boot/Splash]
     │
     ▼
 [Home] ──Start──► [StartPresets] ──Custom──► [StartCustom]
     │                    │                        │
     │                    └──────── Confirm ─────────┘
     │                              │
     │                              ▼
     │                        [Monitoring]
     │                         │    │    │
     │                      Pause Stop Resume
     │                         │    │    │
     │                    [paused] Confirm [drying]
     │                              │
     └◄──────────── Home ◄──────────┘
```

## Screen contracts

### Home
- Shows: status, chamber temp/humidity (if available), **Iniciar Secagem** (≥48×48)
- Optional P2: Settings, History icons

### StartPresets
- Lists `FilamentProfile` from ProfileManager (id, localized name, temp, duration)
- Tap → Confirm dialog with summary
- Affordance: **Personalizado** → StartCustom

### StartCustom
- Steppers/keypad: temp, humidity, duration within validated ranges
- Confirm → start with `profile_id=custom` or explicit params

### Monitoring
- Large live temp/humidity, target, elapsed/remaining, progress
- Actions: **Pausar** / **Retomar** (state-dependent), **Parar**
- P2: tap values to adjust targets

### Dialogs
- Stop confirm: “Tem certeza?” / “Are you sure?”
- Pause: no confirm required (immediate)
- Resume: no confirm required
- Toasts for WS conflict / last-write-wins

## i18n keys (minimum P1)

| Key | PT-BR | EN-US |
|-----|-------|-------|
| `btn_start` | Iniciar Secagem | Start Drying |
| `btn_stop` | Parar | Stop |
| `btn_pause` | Pausar | Pause |
| `btn_resume` | Retomar | Resume |
| `btn_confirm` | Confirmar | Confirm |
| `btn_cancel` | Cancelar | Cancel |
| `title_presets` | Perfis de Filamento | Filament Profiles |

## Accessibility

- Touch targets ≥48×48 px
- High-contrast theme toggle (P2)
- Screen timeout dims backlight; tap restores
