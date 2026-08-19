# Filament Dryer Control App - Cross-Platform Application

## Feature Overview

**Feature Name**: Filament Dryer Control App  
**Short Name**: filament-dryer-control-app  
**Version**: 0.1.0  
**Status**: Draft  
**Date**: 2026-07-23  

## Executive Summary

A cross-platform application (Desktop: Windows/macOS/Linux, Mobile: Android/iOS) for monitoring, configuring, and controlling the Philarmony filament dryer. The app connects to the ESP32 device via WebSocket over the local network, providing a user-friendly interface for real-time status monitoring, drying cycle management, sensor/pin/display configuration, and historical data visualization. Built upon the WebSocket API defined in the ESP32 firmware specification.

## Clarifications

### Session 2026-08-10

- Q: Canonical WebSocket contract (port + topics) — align to 001, dual-protocol, or configurable port with 001 topics only? → A: Spec MUST follow `001` websocket-api: `ws://host/ws` default port 80, `config/hardware`, `max_duration_min`; correct appendix and FRs (no dual-protocol adapter)
- Q: Connection registry entity name vs installer `DeviceProfile` clash? → A: Rename registry entity to `KnownDevice`; installer keeps `DeviceProfile`
- Q: History export scope for MVP — CSV+PDF, CSV only, or CSV+text PDF? → A: CSV **and** PDF both required in MVP (summary + charts in PDF)
- Q: Mobile background WebSocket — full iOS+Android, Android-only/iOS foreground, or none? → A: Require continuous background WS on **iOS and Android** (iOS background mode / entitlements as needed; Android foreground service `dataSync`)
- Q: Pause/Resume vs Stop-only for cycle control in MVP? → A: MVP = **Stop only** (with confirmation); Pause/Resume out of scope until firmware exposes it
- Remediation (analyze I1/I2): SSDP deferred to Out of Scope; discovery acceptance no longer requires Wi‑Fi signal strength unless discovery metadata provides it
- Remediation (analyze C1): tasks MVP gate includes Core UX + bg WS (T075–T078) + CSV/PDF (T060–T069)

## User Scenarios & Testing

> **Test task policy (Constitution v1.0.0):** Journeys, acceptance criteria, and **automated** test-implementation tasks live here. Open **manual** validation (device/UI smoke) → `specs/005-manual-validation`, named by stage. When `tasks.md` is generated, keep automation on this feature; do not leave open manuals here.

### Primary User Scenarios

**Scenario 1: Device Discovery & Connection**
1. User launches app on desktop or mobile
2. App scans via mDNS (`_philarmony._tcp`) or manual IP entry (SSDP out of scope this phase)
3. User selects device from discovered list
4. App establishes WebSocket connection
5. Real-time status dashboard appears with current readings

**Scenario 2: Start Drying Cycle**
1. User connected to device via app
2. User navigates to "Start Drying" screen
3. User sets target temperature (e.g., 50°C), max time (e.g., 2 hours), optional target humidity (e.g., 15%)
4. User taps "Start"
5. App sends control/start command via WebSocket
6. Dashboard updates to "Drying" status with live charts

**Scenario 3: Monitor Active Drying Cycle**
1. Drying cycle in progress
2. Dashboard displays: current temp, target temp, humidity, heater power %, fan status, elapsed/remaining time
3. Real-time charts update every second (temperature curve, humidity curve, heater power)
4. User can **stop** the cycle at any time (confirmation dialog); Pause/Resume **not** offered in MVP
5. Push / local notification when cycle completes or safety triggers (mobile background session MUST remain connected per FR-007 background policy)

**Scenario 4: Configure Device Settings**
1. User opens Settings section
2. Configures sensor types (DHT22, DS18B20, etc.) and GPIO pins
3. Configures display (enabled/disabled, resolution, driver, visible fields)
4. Changes sent via `config/hardware` (and related `config/display` / `config/control` per firmware `001` contract)
5. App confirms settings applied; device reboots if required

**Scenario 5: Historical Data & Reports**
1. User views history screen
2. Lists past drying cycles with: date, material, target temp, actual avg temp, duration, completion reason
3. Exports data as CSV for quality records **and** as PDF report (summary + charts) — both required in MVP
4. Views trend charts across multiple cycles

**Scenario 6: Multi-Device Management**
1. User has multiple dryers (e.g., workshop + home)
2. App stores `KnownDevice` entries for each dryer
3. Quick-switch between devices from dashboard
4. Each device shows connection status (online/offline)

### Acceptance Criteria

| Scenario | Given | When | Then |
|----------|-------|------|------|
| Device discovery | App launched, device on same network | User taps "Scan" | Device list shows ESP32 with display name and IP/hostname (port if non-default); signal strength only if provided by discovery metadata |
| Manual connection | Device not discoverable | User enters IP:port manually | WebSocket connects, dashboard loads |
| Start cycle | Connected, device idle | User enters params, taps Start | Device status changes to "drying", WebSocket stream shows live data |
| Stop cycle | Device drying | User taps Stop | Heater/fan off, status "stopped", cycle saved to history |
| Config sensors | Connected, in Settings | User changes sensor type/pin, saves | Config persisted on device, read back matches |
| Config display | Connected, in Settings | User toggles display, sets resolution/fields | Display updates on device within 2s |
| History export | At least 1 completed cycle | User exports from History as CSV and as PDF | Both files open; PDF includes summary + charts |
| History view | At least 1 completed cycle | User opens History | List shows all cycles with key metrics |
| Multi-device | 2+ devices configured | User switches device | Dashboard updates to selected device's live data |

### Edge Cases

- Network interruption during cycle: App shows "Reconnecting...", buffers commands, resumes on reconnect
- Device in hotspot mode (not on WiFi): App detects, offers to join hotspot and configure WiFi
- Firmware version mismatch: App warns, offers to open installer app for update
- Multiple app instances controlling same device: Last-write-wins for config; status stream shared
- Mobile app backgrounded: WebSocket **MUST** remain connected — Android: foreground service type `dataSync` (or equivalent allowed type); iOS: declared background mode / entitlements required for sustained LAN WS (VoIP or other App Store–acceptable mode as implemented); user MUST be informed if OS denies the capability
- Invalid sensor readings: App shows "Sensor Error" badge, continues with last valid data

## Functional Requirements

### FR-001: Device Discovery & Connection
- Automatic discovery via mDNS (service type `_philarmony._tcp`); manual IP/hostname always available. SSDP deferred (Out of Scope this phase)
- Manual IP/hostname entry with port (default **80**) and path `/ws` → `ws://{host}:{port}/ws` per `specs/001-filament-dryer-esp32/contracts/websocket-api.md`
- Connection status indicator: Connecting / Connected / Disconnected / Error
- Auto-reconnect with exponential backoff (max 60s interval)
- Remember last connected device for auto-connect on launch

### FR-002: Real-Time Status Dashboard
- WebSocket subscription to `status/update` (1Hz updates)
- Display fields per firmware spec: status, chamber_temp_c, target_temp_c, humidity_pct, target_humidity_pct, heater_on, heater_power_pct, exhaust_fan_on, exhaust_fan_power_pct, elapsed_time_sec, remaining_time_sec, cpu_usage_pct, memory_free_bytes, uptime_sec
- Visual indicators: heating/cooling/idle state badges, progress ring for cycle completion
- Live charts: Temperature vs Time, Humidity vs Time, Heater Power vs Time (last 60 minutes, scrolling)
- Unit toggle: °C/°F, %RH, time format

### FR-003: Drying Cycle Control
- Start command per `001`: `profile_id` and/or explicit `target_temp_c` (30-80°C), `max_duration_min` (1-1440), optional `target_humidity_pct` (5-50%); explicit fields override profile when both present
- Stop/Interrupt command: `control/stop` with `reason` (e.g. `user_requested`); immediate heater/fan off
- **Pause/Resume**: **Out of scope for MVP** — do not show Pause/Resume controls until firmware documents support; Stop only
- Safety confirmation dialog for stop (prevent accidental)
- Cycle completion detection: auto-navigate to results screen

### FR-004: Device Configuration
- **Sensors / actuators / display / control**: Edit hardware configuration matching firmware `config/hardware` schema (capabilities, buses, pins, safety limits) — UI may group as Sensors / Pins / Display for usability
- **Pin Mapping**: Visual pinout for selected ESP32 model; drag-drop or dropdown assignment; client MUST validate no GPIO conflicts before send (PinValidator)
- **Display**: Toggle enable, select driver, geometry, fields — via `config/hardware` display section and/or `config/display` as defined in `001`
- **WiFi**: View current SSID, signal strength; option to reconfigure (triggers hotspot mode)
- **Advanced**: Firmware version, device name, NTP timezone, safety temp limit (read-only in v1)
- Acknowledgments via `config/hardware/response` / `config/hardware/error` (and related response topics per `001`); no legacy `config/sensors` / `config/pins` client topics

### FR-005: History & Analytics
- Local SQLite database storing completed cycles
- Cycle record: id, device_id, start_time, end_time, target_temp_c, avg_temp_c, max_temp_c, target_humidity_pct, avg_humidity_pct, duration_sec, stop_reason, material_name (user tag), notes
- List view with sorting/filtering (date range, material, device)
- Detail view with full charts for the cycle
- Export (**MVP required**): CSV (all fields) **and** PDF report (summary + charts); both formats MUST be available from History detail / list actions
- Statistics: total cycles, total hours, avg cycle time, success rate

### FR-006: Multi-Device Management
- Device registry (`KnownDevice`): nickname, IP/hostname, last_seen, firmware_version, auto_connect flag
- Quick switcher in header/sidebar
- Simultaneous WebSocket connections (configurable max, default 3)
- Per-device notification settings

### FR-007: Notifications & Alerts
- **In-app**: Toast/banner for cycle complete, safety cutoff, sensor error, connection lost
- **Background session (MVP required)**: While a drying cycle is active (or user opted to keep monitoring), mobile apps MUST sustain WebSocket in background — Android foreground service; iOS background networking entitlement/mode as required for App Store submission
- **Mobile notifications**: Local (and optional remote later) alerts for cycle complete / errors; require notification permission; MUST work when app is backgrounded if OS grants background session
- **Desktop**: System notifications (Windows Toast, macOS Notification Center, Linux libnotify)
- Configurable per event type

### FR-007a: Background Connectivity (Mobile)
- Sustained background WS is an **acceptance requirement** for MVP on Android and iOS when an active cycle is monitored
- Document required entitlements in app README / store privacy disclosures
- If the OS revokes background execution, app MUST surface a clear error and attempt graceful reconnect on foreground

### FR-008: Material Profiles (Presets)
- Sync with firmware filament profiles via WebSocket (`config/profiles/list`, `config/profiles/get`, `config/profiles/create`, `config/profiles/update`, `config/profiles/delete`, `config/profiles/reset_defaults`)
- Built-in defaults from firmware: PLA (50°C, 4h, 15% RH), PETG (65°C, 4h, 15% RH), ABS (80°C, 2h, 10% RH), TPU (45°C, 4h, 20% RH), Nylon (70°C, 6h, 10% RH)
- Each profile has Portuguese and English display names
- User can create/edit/delete custom profiles (synced to device)
- One-tap "Start with Profile" from dashboard sends `control/start` with `profile_id`
- Profile validation: temp 30-80°C, duration 1-1440 min, humidity 5-50%
- Offline editing queued for sync on next connection

### FR-009: Localization & Accessibility
- Languages: Portuguese (BR) primary, English (US) secondary
- RTL support ready (for future)
- Dynamic text sizing, VoiceOver/TalkBack labels, high contrast mode
- Keyboard navigation (desktop), focus management

### FR-010: Offline & Sync
- Full offline capability: view history, edit profiles, configure settings (queued for next connection)
- Pending commands queue with visual indicator
- Conflict resolution: server (device) wins for config; local history merges by timestamp

## Non-Functional Requirements

### Performance
- Dashboard first paint: <1.5s after connection
- Chart render/update: 60fps scrolling, <16ms per frame
- WebSocket message processing: <10ms per message
- App launch to interactive: <3s cold, <1s warm
- Memory: <150MB desktop, <100MB mobile

### Reliability
- WebSocket reconnection: 99.9% within 10s of network restore
- Zero data loss for history (local-first architecture)
- Crash-free rate: >99.9% sessions

### Usability
- New user completes first drying cycle without documentation: >90%
- Task completion time (start cycle): <30s from app launch
- System Usability Scale (SUS) target: >80

### Compatibility
- Desktop: Windows 10/11 (x64), macOS 12+ (Universal), Linux (Ubuntu 20.04+, AppImage/Flatpak)
- Mobile: Android 8+ (API 26), iOS 15+
- Screen sizes: 360dp+ width (mobile), 800px+ (desktop)
- Orientation: Portrait/landscape adaptive

### Security
- No authentication in v1 (local network only)
- Optional: PIN lock for app access
- Cleartext WebSocket (matches firmware); TLS in future
- No cloud connectivity in v1

## Key Entities

### KnownDevice (connection registry; formerly referred to as "DeviceProfile" in this feature only)

> **Terminology**: Do **not** confuse with installer `DeviceProfile` in `philarmony_core` (flash/wizard hardware+WiFi profile). Control-app saved connection targets are always `KnownDevice`.

- `id`: UUID
- `nickname`: string
- `host`: string (IP or hostname)
- `port`: number (default **80**; WebSocket path `/ws`)
- `path`: string (default `/ws`)
- `device_model`: string (from firmware)
- `firmware_version`: string
- `last_connected`: timestamp
- `auto_connect`: boolean
- `notification_settings`: object

### DryingCycle
- `id`: UUID
- `device_id`: UUID (FK → `KnownDevice.id`)
- `profile_id`: UUID (nullable, FK)
- `material_name`: string
- `target_temp_c`: number
- `max_duration_sec`: number
- `target_humidity_pct`: number (nullable)
- `start_time`: timestamp
- `end_time`: timestamp (nullable)
- `avg_temp_c`: number
- `max_temp_c`: number
- `avg_humidity_pct`: number
- `stop_reason`: enum (completed, stopped, target_humidity, max_time, safety_cutoff, error, power_loss)
- `notes`: string
- `data_points`: JSON array (timestamp, temp, humidity, heater_pct, fan_pct) - optional, for charts

### MaterialProfile
- `id`: string (builtin: pla, petg, abs, tpu, nylon; custom: UUID)
- `name_pt`: string (Portuguese display name)
- `name_en`: string (English display name)
- `target_temp_c`: number
- `default_duration_min`: number
- `target_humidity_pct`: number (nullable)
- `is_builtin`: boolean (read-only, true for default profiles)
- `created_at`: timestamp (for custom profiles)
- `updated_at`: timestamp (for custom profiles)
- Synced with firmware via WebSocket profile topics

### DeviceConfig (cached from device)
- `sensors`: SensorConfig[]
- `pin_mapping`: PinMapping
- `display`: DisplayConfig
- `wifi_ssid`: string
- `device_name`: string
- `firmware_version`: string
- `safety_temp_limit_c`: number

### SensorConfig
- `type`: enum (temperature: dht22, ds18b20, bme280, none; humidity: dht22, bme280, none)
- `gpio_pin`: number
- `parameters`: object

### PinMapping
- `heater_pwm`: number
- `exhaust_fan_pwm`: number (nullable)
- `exhaust_fan_digital`: number (nullable)
- `i2c_sda`: number (nullable)
- `i2c_scl`: number (nullable)
- `spi_mosi`: number (nullable)
- `spi_miso`: number (nullable)
- `spi_sck`: number (nullable)
- `spi_cs`: number (nullable)

### DisplayConfig
- `enabled`: boolean
- `driver`: enum (ssd1306, sh1106, st7789, ili9341, auto)
- `width`: number
- `height`: number
- `i2c_address`: number (nullable)
- `spi_dc`: number (nullable)
- `spi_reset`: number (nullable)
- `fields`: string[] (subset of status payload keys)

## Success Criteria

| Metric | Target | Measurement |
|--------|--------|-------------|
| Connection success rate | >98% | Successful WebSocket handshakes / attempts |
| Dashboard load time | <2s | Connection established → first status render |
| Chart frame rate | 60fps | Scrolling 60min window, no dropped frames |
| Cycle start latency | <500ms | Tap Start → WebSocket command sent |
| History CSV+PDF export | 100% MVP | Both formats succeed for a completed cycle in acceptance test |
| Cross-platform feature parity | 100% | Feature matrix checklist |
| Offline usability | Full | All read operations work offline |
| Localization completeness | 100% PT-BR/EN-US | All strings translated |
| Accessibility score | WCAG 2.1 AA | Automated + manual audit |
| Mobile background WS | Sustained while cycle active | Background app ≥2 min on Android and iOS; status still updates / reconnects per policy |

## Assumptions

1. Firmware WebSocket API is **normative** from `specs/001-filament-dryer-esp32/contracts/websocket-api.md` (topics, payloads, framing, default `ws://host/ws` port 80)
2. Device runs on same local network (no VPN/relay needed in v1)
3. ESP32 mDNS hostname: `philarmony-[mac_suffix].local` or fixed service name
4. Firmware version ≥0.1.0 with config NVS schema v1
5. No authentication/encryption on WebSocket in v1 (local trusted network)
6. Mobile platforms **MUST** support sustained background WebSocket for active-cycle monitoring (Android foreground service; iOS background mode/entitlements). Store review risk is accepted; implementation MUST follow platform guidelines.
7. Desktop installer (`002-esp32-desktop-installer`) can launch this app post-flash
8. User has basic 3D printing knowledge (materials, temps)
9. SQLite suitable for local history (single-user, no sync in v1)
10. Push notifications optional, require backend service (deferred to v2)

## Dependencies & Constraints

- **Firmware coupling**: App must match firmware WebSocket protocol version (semantic versioning)
- **Platform SDKs**: Flutter single codebase (desktop + mobile) per product direction / DEC-010/012 — alternatives (React Native, Tauri, MAUI) rejected for this feature
- **mDNS/Bonjour**: Requires network permissions (local network entitlement on iOS 14+)
- **Google Play**: Android 14+ target SDK; foreground service type `dataSync` (or documented equivalent) for background WebSocket — **required** for MVP
- **App Store**: iOS privacy manifest + local network usage; background mode/entitlements for sustained WS — **required** for MVP (choose App Store–acceptable mode; document rationale)
- **Microsoft Store / Mac App Store**: Sandboxing considerations for local DB
- **GPLv3**: App source must be open (matches firmware license)
- **Constitutional**: PT-BR/EN-US docs, safety validation for config changes

## Out of Scope (This Phase)

- Cloud synchronization of profiles/history
- Multi-user accounts / roles
- OTA firmware update from app
- Real-time video/camera integration
- Filament weight scale integration
- Scheduler / delayed start
- Shared device access (family/team)
- Advanced analytics / ML drying optimization
- Plugin system for custom sensors
- Web-based dashboard (separate project)
- Cycle **Pause/Resume** (until firmware exposes a documented control topic)
- SSDP / UPnP discovery (mDNS + manual only in this phase)

## Appendix: WebSocket Message Quick Reference

**Canonical source**: `specs/001-filament-dryer-esp32/contracts/websocket-api.md` (this appendix is a non-normative summary; on conflict, `001` wins).

**Transport**: `ws://{host}/ws` (default port **80**)

**Client → Server (examples)**
```json
{ "topic": "status/subscribe", "payload": {} }
{ "topic": "control/start", "payload": { "profile_id": "pla", "target_temp_c": 50.0, "max_duration_min": 120, "target_humidity_pct": 20.0 } }
{ "topic": "control/stop", "payload": { "reason": "user_requested" } }
{ "topic": "config/hardware", "payload": { "sensors": [], "actuators": [], "display": {}, "control": {} } }
```

**Server → Client (1Hz)**
```json
{ "topic": "status/update", "payload": { "status": "drying", "chamber_temp_c": 48.5, "target_temp_c": 50, "humidity_pct": 22.1, "target_humidity_pct": 20, "heater_on": true, "heater_power_pct": 65, "exhaust_fan_on": true, "exhaust_fan_power_pct": 80, "elapsed_time_sec": 3600, "remaining_time_sec": 3600, "cpu_usage_pct": 12.5, "memory_free_bytes": 245760, "uptime_sec": 7200 } }
```

**Config Acknowledgment / Error**
```json
{ "topic": "config/hardware/response", "payload": { "status": "saved" } }
{ "topic": "config/hardware/error", "payload": { "error": "Pin conflict: GPIO 25 already used by heater" } }
```