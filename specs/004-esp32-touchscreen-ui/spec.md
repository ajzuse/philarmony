# ESP32 Touchscreen Interface - On-Device Touch UI

## Feature Overview

**Feature Name**: ESP32 Touchscreen Interface  
**Short Name**: esp32-touchscreen-ui  
**Version**: 0.1.0  
**Status**: Draft  
**Date**: 2026-07-23  

## Executive Summary

Enhance the ESP32 filament dryer firmware with a touch-enabled user interface for display models equipped with touchscreen capability (resistive or capacitive). The on-device UI allows users to start/stop drying cycles, adjust target temperature and humidity, and perform basic configuration directly on the device without requiring the mobile/desktop app or WebSocket connection. This feature builds upon the existing display infrastructure from the base firmware specification.

## User Scenarios & Testing

> **Test task policy (Constitution v1.0.0):** Journeys, acceptance criteria, and **automated** test-implementation tasks live here. Open **manual** validation (on-device UI smoke) → `specs/005-manual-validation`, named by stage. When `tasks.md` is generated, keep automation on this feature; do not leave open manuals here.

### Primary User Scenarios

**Scenario 1: Start Drying Cycle via Touchscreen**
1. User approaches dryer with touchscreen display
2. Home screen shows current status (idle, temperature, humidity)
3. User taps "Iniciar Secagem" (Start Drying) button
4. Quick-set screen appears with preset material buttons (PLA, PETG, ABS, TPU, Custom)
5. User selects material or taps "Personalizado" for manual entry
6. For custom: user adjusts target temperature (± buttons or numeric keypad), target humidity, max time
7. User taps "Confirmar" → drying starts, screen transitions to active monitoring view

**Scenario 2: Stop Active Drying Cycle**
1. Drying cycle in progress, monitoring screen displayed
2. Large "Parar" (Stop) button visible at bottom
3. User taps "Parar" → confirmation dialog appears ("Tem certeza?")
4. User confirms → heater/fan stop, status returns to idle, cycle saved to history

**Scenario 2b: Pause and Resume Drying Cycle**
1. Drying cycle in progress, monitoring screen displayed
2. User taps "Pausar" (Pause) — no confirmation required
3. Heater cuts off within 500ms, elapsed timer freezes, status shows paused
4. User taps "Retomar" (Resume) → control loop resumes with same profile/targets
5. Stop remains available while paused (with confirmation)

**Scenario 3: Adjust Target Temperature/Humidity Mid-Cycle**
1. Drying cycle running
2. User taps temperature/humidity value on monitoring screen
3. Adjustment overlay appears with ± steppers or keypad
4. User modifies target temperature (30-80°C) and/or target humidity (5-50%)
5. User taps "Aplicar" → new targets sent to control loop immediately

**Scenario 4: Basic Configuration via Touchscreen**
1. From home screen, user taps "Configurações" (gear icon)
2. Settings menu: WiFi status, Display brightness, Temperature unit (°C/°F), Language (PT/EN), Sensor calibration offset
3. User modifies setting → changes applied immediately, persisted to NVS
4. "Resetar Configurações" option with confirmation

**Scenario 5: View Cycle History on Device**
1. From home screen, user taps "Histórico"
2. List of last 10 cycles with: date, material, target temp, duration, result
3. Tap cycle for details: temperature/humidity curves (simplified sparklines), avg/max values

### Acceptance Criteria

| Scenario | Given | When | Then |
|----------|-------|------|------|
| Start cycle | Device idle, touchscreen active | User completes start flow | Drying begins, monitoring screen shows live data |
| Stop cycle | Drying in progress or paused | User taps Stop, confirms | Heater/fan off within 500ms, status "stopped" |
| Pause cycle | Drying in progress | User taps Pause | Heater off within 500ms, status "paused", timer frozen |
| Resume cycle | Cycle paused | User taps Resume | Status "drying", same targets, timer resumes |
| Adjust targets | Drying in progress | User changes temp/humidity targets | Control loop receives new targets within 1s |
| Configure settings | In settings menu | User changes brightness/unit/language | Change applied immediately, persists reboot |
| View history | At least 1 completed cycle | User opens History | List shows cycles with key metrics |

### Edge Cases

- Touch during screen transition: Ignore or queue until stable
- Invalid touch coordinates (edge noise): Filter via debounce/radius
- Screen timeout during config: Auto-save and return to monitoring
- Power loss during cycle: Resume prompt on boot ("Continuar ciclo anterior?")
- Touchscreen calibration drift: Recalibration option in settings
- Gloved fingers / wet hands: Adjust touch sensitivity setting
- Simultaneous WebSocket command + touch input: Last-write-wins with visual feedback

## Functional Requirements

### FR-001: Touch Input Handling
- Support for resistive (XPT2046, STMPE610) and capacitive (FT6236, GT911, CST816S) touch controllers
- Touch driver auto-detection via I2C/SPI probe during display init
- Configurable touch sensitivity (Low/Medium/High) for gloved/wet operation
- Debounce: 50ms press, 100ms release, minimum 8px movement for drag
- Calibration routine: 4-point calibration stored in NVS, re-runnable from settings

### FR-002: UI Framework & Rendering
- Lightweight immediate-mode or retained-mode GUI library (e.g., LVGL, custom)
- 30fps minimum render rate, 60fps target on capable displays (ST7789, ILI9341)
- Double-buffered rendering to prevent tearing
- Theme: Dark mode default, high contrast for visibility in workshop lighting
- Font: Noto Sans / Roboto subset, sizes 12/16/24/32pt for labels/values/buttons
- Portuguese (BR) primary, English (US) secondary - all strings externalized

### FR-003: Screen Navigation & State Machine
- **Home/Idle**: Status summary, large Start button, Settings/History icons
- **Start Flow**: Material presets → Custom params → Confirm
- **Monitoring**: Live values (large), progress ring, Stop button, tap values to adjust
- **Settings**: Categorized list (Rede, Tela, Unidades, Sensores, Avançado)
- **History**: Scrollable list, tap for detail modal
- **Dialogs**: Confirmation, numeric keypad, message/toast
- Navigation: Hardware back gesture (swipe from left) or on-screen back button
- Screen timeout: Configurable (30s-10min, default 2min), dims to 10% brightness

### FR-004: Drying Cycle Control via Touch
- **Start**: Validate targets (temp 30-80°C, humidity 5-50%, time 1-1440min), send internal start command via ProfileManager (preset `profile_id` or custom params)
- **Stop**: Immediate heater/fan cutoff, show confirmation dialog, log stop_reason="user_stopped" (allowed from drying or paused)
- **Adjust Targets**: Increment/decrement (±1°C, ±1%RH, ±5min) or numeric keypad, apply to running control loop
- **Pause/Resume**: MVP — `PAUSED` state cuts heater within 500ms, freezes elapsed timer; Resume returns to DRYING with same session; expose `control/pause` / `control/resume` for WS sync (control app 003 may remain Stop-only until it adopts)
- Material Presets: PLA(50°C/4h/15%), PETG(65°C/4h/15%), ABS(80°C/2h/10%), TPU(45°C/4h/20%), Nylon(70°C/6h/10%), Custom — sourced from ProfileManager / NVS, not a duplicate UI table

### FR-005: On-Device Configuration
- **WiFi**: Show current SSID, signal (RSSI bars), "Reconfigurar" → triggers hotspot mode
- **Display**: Brightness slider (0-100%), timeout selector, orientation (0/90/180/270°)
- **Units**: Temperature °C/°F toggle, applied to all displays and WebSocket
- **Language**: PT-BR / EN-US, immediate UI refresh
- **Sensors**: View current sensor types/pins (read-only in v1, config via app/installer)
- **Touch**: Sensitivity (Low/Med/High), recalibration wizard
- **Advanced**: Device name, safety temp limit (read-only), firmware version, reset to defaults

### FR-006: History & Data Visualization
- Local history stored in NVS/spiffs: last 50 cycles (circular buffer)
- Each record: timestamp, material, target_temp, avg_temp, max_temp, target_humidity, avg_humidity, duration_sec, stop_reason
- List view: Date, Material badge, Target temp, Duration, Result icon (✓/✗/⚠)
- Detail view: Sparkline charts (temp/humidity over time), statistics cards
- Export: "Enviar para App" via WebSocket (when connected), or CSV to SD card (if present)

### FR-007: Synchronization with WebSocket/App
- Touch UI and WebSocket API share same internal state
- Commands from either source update state, both UIs reflect changes
- WebSocket `status/update` includes `ui_source`: "touch" | "websocket" | "auto"
- Config changes via touch broadcast via `config/*/ack` to connected clients
- Conflict resolution: Last command wins, visual feedback on both ends

### FR-008: Accessibility & Usability
- Large touch targets: minimum 48x48px (40x40dp)
- High contrast mode toggle in settings
- Haptic feedback (vibration motor if present) or audio beep on press
- Screen reader compatible labels (for future TTS)
- Lock screen: Optional PIN to prevent accidental changes during drying

## Non-Functional Requirements

### Performance
- Touch response latency: <50ms (press to visual feedback)
- Screen transition animation: <200ms
- Boot to interactive UI: <3s (including touch init)
- Memory overhead: <200KB RAM for UI framebuffers + assets
- CPU usage: <10% additional when UI active

### Reliability
- Touch controller watchdog: reset if no interrupts for 5s
- UI task watchdog: reset if frame time >100ms
- NVS write endurance: batch config writes, wear leveling
- Power loss during write: atomic operations, checksum validation

### Usability
- New user starts cycle without manual: >95% success rate
- Gloved hand operation: success with "High" sensitivity
- Bright sunlight readability: 400+ nits typical, auto-brightness if ALS present
- One-handed operation: key controls in lower 2/3 of screen

## Key Entities

### TouchConfig
- `controller_type`: enum (xpt2046, stmpe610, ft6236, gt911, cst816s, auto)
- `i2c_address` / `spi_cs`: number
- `irq_pin`: number (optional)
- `calibration_data`: { x_min, x_max, y_min, y_max, swapped_xy }
- `sensitivity`: enum (low, medium, high)
- `swap_xy`: boolean
- `invert_x`: boolean
- `invert_y`: boolean

### UIScreen (state machine)
- `current`: enum (home, start_material, start_custom, monitoring, settings_main, settings_wifi, settings_display, settings_units, settings_touch, settings_advanced, history_list, history_detail, dialog_confirm, dialog_keypad, dialog_message)
- `previous`: enum (for back navigation)
- `params`: object (context data for current screen)

### MaterialPreset
- `id`: string (pla, petg, abs, tpu, nylon, custom)
- `name_pt`: string
- `name_en`: string
- `target_temp_c`: number
- `default_duration_min`: number
- `target_humidity_pct`: number

### CycleRecord (on-device)
- `id`: number (incremental)
- `timestamp`: unix_ms
- `material_id`: string
- `target_temp_c`: number
- `avg_temp_c`: number
- `max_temp_c`: number
- `target_humidity_pct`: number
- `avg_humidity_pct`: number
- `duration_sec`: number
- `stop_reason`: enum (completed, stopped, target_humidity, max_time, safety_cutoff, error, power_loss)

### UISettings
- `brightness_pct`: number (10-100)
- `timeout_sec`: number (30-600)
- `orientation`: enum (0, 90, 180, 270)
- `temp_unit`: enum (celsius, fahrenheit)
- `language`: enum (pt_br, en_us)
- `touch_sensitivity`: enum (low, medium, high)
- `high_contrast`: boolean
- `pin_lock_enabled`: boolean
- `pin_hash`: string (bcrypt, optional)

## Success Criteria

| Metric | Target | Measurement |
|--------|--------|-------------|
| Touch response latency | <50ms | Press to visual feedback (LED/button state) |
| Boot to interactive UI | <3s | Power on to home screen responsive |
| Frame rate | 30fps min, 60fps target | LVGL/UI task frame time |
| Memory overhead | <200KB | Additional RAM vs headless firmware |
| Cycle start via touch | <3 taps | Home → Material → Confirm |
| Settings change persistence | 100% | Survives power cycle |
| History retention | 50 cycles | Circular buffer in NVS/SPIFFS |
| Gloved hand success rate | >90% | With "High" sensitivity |
| WebSocket sync latency | <100ms | Touch action → WebSocket broadcast |

## Assumptions

1. Display hardware supports touch (resistive overlay or capacitive panel)
2. Touch controller shares I2C/SPI bus with display or uses dedicated pins
3. Base firmware (`001-filament-dryer-esp32`) provides control loop, NVS, WebSocket
4. LVGL v8+ or similar GUI library integrated into firmware build
5. Font assets stored in SPIFFS or embedded in flash (~200KB)
6. Touch calibration survives reboot (stored in NVS)
7. No multi-touch gestures required (single touch only)
8. Screen resolution ≥240x240 for usable touch targets
9. WebSocket and touch UI can coexist without conflict
10. PIN lock uses simple hash verification (no secure element)

## Dependencies & Constraints

- **Firmware coupling**: Requires base firmware v0.2.0+ with UI task integration
- **Display drivers**: Must support touch interrupt or polling
- **Memory**: Additional 200KB RAM + 500KB flash for GUI assets
- **Build system**: LVGL configuration via menuconfig/Kconfig
- **GPLv3**: UI code open source, fonts licensed for redistribution
- **Constitutional**: PT-BR/EN-US strings, safety validation for touch-initiated commands

## Out of Scope (This Phase)

- Multi-touch gestures (pinch, swipe, drag)
- On-screen keyboard for arbitrary text (numeric keypad only)
- Custom theme editor
- Animation/transition designer
- Plugin system for custom screens
- Voice control / TTS
- Remote UI mirroring (VNC/WebView)
- Firmware update from touch UI

## Appendix: Screen Flow Diagram

```
[Boot] → [Splash] → [Home/Idle]
                    ↓
         ┌──────────┼──────────┐
         ↓          ↓          ↓
    [Start]    [Settings]  [History]
      ↓           ↓           ↓
   [Presets]  [Categories]  [List]
      ↓           ↓           ↓
   [Custom]   [Options]    [Detail]
      ↓           ↓
   [Confirm]  [Save]
      ↓
[Monitoring] ←←←←←←←←←←←←←←
      ↓
   [Pause] ⇄ [Paused]
      ↓
    [Stop] → [Confirm] → [Home]
```

## Appendix: Touch Event → Internal Command Mapping

| Touch Action | Internal Command | WebSocket Equivalent |
|--------------|------------------|---------------------|
| Tap "Iniciar" → Preset → Confirm | `dryer_start(preset)` | `control/start` |
| Tap "Personalizado" → Set values → Confirm | `dryer_start(custom)` | `control/start` |
| Tap "Pausar" | `dryer_pause(user)` | `control/pause` |
| Tap "Retomar" | `dryer_resume()` | `control/resume` |
| Tap "Parar" → Confirm | `dryer_stop(user)` | `control/stop` |
| Tap temp value → Adjust → Apply | `dryer_set_target(temp)` | N/A (local only) |
| Tap humidity value → Adjust → Apply | `dryer_set_target(humidity)` | N/A (local only) |
| Settings → Brightness → Save | `config_display(brightness)` | `config/display` |
| Settings → Language → Save | `config_language(lang)` | N/A (local only) |
| Settings → Touch Calibrate | `touch_calibrate()` | N/A |
| History → Tap cycle | `history_show(id)` | N/A |