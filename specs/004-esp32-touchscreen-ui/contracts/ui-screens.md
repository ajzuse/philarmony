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
- Settings and History icons **required in v0.1**

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
- Tap values to adjust targets (v0.1); on Apply → control loop + WS broadcast

### History
- Initial list: newest 10; **Mais** loads older up to 50 stored

### Dialogs
- Stop confirm: “Tem certeza?” / “Are you sure?”
- Pause: no confirm required (immediate)
- Resume: no confirm required
- Pause timeout toast after 30 min auto-stop
- Toasts for WS conflict / last-write-wins

## i18n keys (minimum v0.1)

| Key | PT-BR | EN-US |
|-----|-------|-------|
| `btn_start` | Iniciar Secagem | Start Drying |
| `btn_stop` | Parar | Stop |
| `btn_pause` | Pausar | Pause |
| `btn_resume` | Retomar | Resume |
| `btn_confirm` | Confirmar | Confirm |
| `btn_cancel` | Cancelar | Cancel |
| `btn_more` | Mais | More |
| `title_presets` | Perfis de Filamento | Filament Profiles |
| `title_history` | Histórico | History |
| `title_settings` | Configurações | Settings |

## Accessibility

- Touch targets ≥48×48 px
- High-contrast theme toggle (P2)
- Screen timeout dims backlight; tap restores
