# Filament Dryer Control App - Cross-Platform Application

## Feature Overview

**Feature Name**: Filament Dryer Control App  
**Short Name**: filament-dryer-control-app  
**Version**: 0.1.0  
**Status**: Draft  
**Date**: 2026-07-23  

## Executive Summary

A cross-platform application (Desktop: Windows/macOS/Linux, Mobile: Android/iOS) for monitoring, configuring, and controlling the Philarmony filament dryer. The app connects to the ESP32 device via WebSocket over the local network, providing a user-friendly interface for real-time status monitoring, drying cycle management, sensor/pin/display configuration, and historical data visualization. Built upon the WebSocket API defined in the ESP32 firmware specification.

## User Scenarios & Testing

### Primary User Scenarios

**Scenario 1: Device Discovery & Connection**
1. User launches app on desktop or mobile
2. App scans local network for Philarmony devices (mDNS/SSDP or manual IP entry)
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
4. User can pause/resume or stop cycle at any time
5. Push notification (mobile) when cycle completes or safety triggers

**Scenario 4: Configure Device Settings**
1. User opens Settings section
2. Configures sensor types (DHT22, DS18B20, etc.) and GPIO pins
3. Configures display (enabled/disabled, resolution, driver, visible fields)
4. Changes sent via config/sensors, config/pins, config/display topics
5. App confirms settings applied; device reboots if required

**Scenario 5: Historical Data & Reports**
1. User views history screen
2. Lists past drying cycles with: date, material, target temp, actual avg temp, duration, completion reason
3. Exports data as CSV for quality records
4. Views trend charts across multiple cycles

**Scenario 6: Multi-Device Management**
1. User has multiple dryers (e.g., workshop + home)
2. App stores connection profiles for each device
3. Quick-switch between devices from dashboard
4. Each device shows connection status (online/offline)

### Acceptance Criteria

| Scenario | Given | When | Then |
|----------|-------|------|------|
| Device discovery | App launched, device on same network | User taps "Scan" | Device list shows ESP32 with name, IP, signal strength |
| Manual connection | Device not discoverable | User enters IP:port manually | WebSocket connects, dashboard loads |
| Start cycle | Connected, device idle | User enters params, taps Start | Device status changes to "drying", WebSocket stream shows live data |
| Stop cycle | Device drying | User taps Stop | Heater/fan off, status "stopped", cycle saved to history |
| Config sensors | Connected, in Settings | User changes sensor type/pin, saves | Config persisted on device, read back matches |
| Config display | Connected, in Settings | User toggles display, sets resolution/fields | Display updates on device within 2s |
| History view | At least 1 completed cycle | User opens History | List shows all cycles with key metrics |
| Multi-device | 2+ devices configured | User switches device | Dashboard updates to selected device's live data |

### Edge Cases

- Network interruption during cycle: App shows "Reconnecting...", buffers commands, resumes on reconnect
- Device in hotspot mode (not on WiFi): App detects, offers to join hotspot and configure WiFi
- Firmware version mismatch: App warns, offers to open installer app for update
- Multiple app instances controlling same device: Last-write-wins for config; status stream shared
- Mobile app backgrounded: WebSocket maintained (iOS voip/background mode, Android foreground service)
- Invalid sensor readings: App shows "Sensor Error" badge, continues with last valid data

## Functional Requirements

### FR-001: Device Discovery & Connection
- Automatic discovery via mDNS (service type `_philarmony._tcp`) and/or SSDP
- Manual IP/hostname entry with port (default 8080)
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
- Start command: target_temp_c (required, 30-80°C), max_time_min (required, 1-1440), target_humidity_pct (optional, 5-50%)
- Stop/Interrupt command: immediate heater/fan off
- Pause/Resume: if firmware supports (future), otherwise Stop only
- Safety confirmation dialog for stop (prevent accidental)
- Cycle completion detection: auto-navigate to results screen

### FR-004: Device Configuration
- **Sensors**: Select type per sensor (temperature: DHT22, DS18B20, BME280, None; humidity: DHT22, BME280, None), assign GPIO pins, validate no conflicts
- **Pin Mapping**: Visual pinout for selected ESP32 model; drag-drop or dropdown assignment for heater_pwm, fan_pwm, fan_digital, sensor pins, I2C/SPI pins
- **Display**: Toggle enable, select driver (SSD1306, SH1106, ST7789, ILI9341, Auto), resolution preset or custom, I2C address/SPI pins, field selection checklist from status payload
- **WiFi**: View current SSID, signal strength; option to reconfigure (triggers hotspot mode)
- **Advanced**: Firmware version, device name, NTP timezone, safety temp limit (read-only in v1)
- All config changes sent via respective WebSocket topics with acknowledgment

### FR-005: History & Analytics
- Local SQLite database storing completed cycles
- Cycle record: id, device_id, start_time, end_time, target_temp_c, avg_temp_c, max_temp_c, target_humidity_pct, avg_humidity_pct, duration_sec, stop_reason, material_name (user tag), notes
- List view with sorting/filtering (date range, material, device)
- Detail view with full charts for the cycle
- Export: CSV (all fields), PDF report (summary + charts)
- Statistics: total cycles, total hours, avg cycle time, success rate

### FR-006: Multi-Device Management
- Device registry: nickname, IP/hostname, last_seen, firmware_version, auto_connect flag
- Quick switcher in header/sidebar
- Simultaneous WebSocket connections (configurable max, default 3)
- Per-device notification settings

### FR-007: Notifications & Alerts
- **In-app**: Toast/banner for cycle complete, safety cutoff, sensor error, connection lost
- **Mobile Push** (Android FCM, iOS APNs via local push): Cycle complete, error alerts (optional, requires notification permission)
- **Desktop**: System notifications (Windows Toast, macOS Notification Center, Linux libnotify)
- Configurable per event type

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

### DeviceProfile
- `id`: UUID
- `nickname`: string
- `host`: string (IP or hostname)
- `port`: number (default 8080)
- `device_model`: string (from firmware)
- `firmware_version`: string
- `last_connected`: timestamp
- `auto_connect`: boolean
- `notification_settings`: object

### DryingCycle
- `id`: UUID
- `device_id`: UUID (FK)
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
| History query (100 cycles) | <200ms | Local DB query + render |
| Cross-platform feature parity | 100% | Feature matrix checklist |
| Offline usability | Full | All read operations work offline |
| Localization completeness | 100% PT-BR/EN-US | All strings translated |
| Accessibility score | WCAG 2.1 AA | Automated + manual audit |
| Crash-free sessions | >99.9% | Sentry/crashlytics data |

## Assumptions

1. Firmware WebSocket API matches spec from `001-filament-dryer-esp32` (topics, payloads, 1Hz status)
2. Device runs on same local network (no VPN/relay needed in v1)
3. ESP32 mDNS hostname: `philarmony-[mac_suffix].local` or fixed service name
4. Firmware version ≥0.1.0 with config NVS schema v1
5. No authentication/encryption on WebSocket in v1 (local trusted network)
6. Mobile platforms allow background WebSocket (iOS: voip background mode; Android: foreground service)
7. Desktop installer (`002-esp32-desktop-installer`) can launch this app post-flash
8. User has basic 3D printing knowledge (materials, temps)
9. SQLite suitable for local history (single-user, no sync in v1)
10. Push notifications optional, require backend service (deferred to v2)

## Dependencies & Constraints

- **Firmware coupling**: App must match firmware WebSocket protocol version (semantic versioning)
- **Platform SDKs**: Flutter 3.x / React Native 0.7x / Tauri 2.x / .NET MAUI - single codebase preferred
- **mDNS/Bonjour**: Requires network permissions (local network entitlement on iOS 14+)
- **App Store**: iOS requires privacy manifest, network usage description
- **Google Play**: Android 14+ target SDK, foreground service type `dataSync` for WebSocket
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

## Appendix: WebSocket Message Quick Reference

Based on `001-filament-dryer-esp32` spec:

**Client → Server**
```json
{ "topic": "config/sensors", "payload": { "temperature": {"type": "dht22", "pin": 4}, "humidity": {"type": "dht22", "pin": 4} } }
{ "topic": "config/pins", "payload": { "heater_pwm": 25, "exhaust_fan_pwm": 26, "exhaust_fan_digital": 27 } }
{ "topic": "config/display", "payload": { "enabled": true, "width": 128, "height": 64, "driver": "ssd1306", "fields": ["chamber_temp_c", "humidity_pct", "heater_power_pct", "status"] } }
{ "topic": "control/start", "payload": { "target_temp_c": 50, "max_time_min": 120, "target_humidity_pct": 20 } }
{ "topic": "control/stop", "payload": {} }
{ "topic": "status/subscribe", "payload": {} }
```

**Server → Client (1Hz)**
```json
{ "topic": "status/update", "payload": { "status": "drying", "chamber_temp_c": 48.5, "target_temp_c": 50, "humidity_pct": 22.1, "target_humidity_pct": 20, "heater_on": true, "heater_power_pct": 65, "exhaust_fan_on": true, "exhaust_fan_power_pct": 80, "elapsed_time_sec": 3600, "remaining_time_sec": 3600, "cpu_usage_pct": 12.5, "memory_free_bytes": 245760, "uptime_sec": 7200 } }
```

**Config Acknowledgment**
```json
{ "topic": "config/sensors/ack", "payload": { "success": true, "message": "Sensors configured", "requires_reboot": false } }
```