# NVS / Config Mapping — Installer → Firmware 001

**Goal**: Transform `DeviceProfile` into boot-time config that firmware `ConfigManager` / `HardwareConfigParser` already understand.

## Principles

1. Prefer the same JSON shapes as `specs/001-filament-dryer-esp32/contracts/config-schema.json` and HTTP/WS hardware payloads.
2. Do not invent parallel key names; map installer UI fields → firmware keys.
3. WiFi credentials written only to device NVS at flash time — never leave plaintext passwords in exported profiles.

## Field map

| Installer | Firmware destination |
|-----------|----------------------|
| `wifi.ssid` / `wifi.password` | NVS WiFi config (`ConfigManager` wifi keys) |
| `sensors[]` + `pin_mapping` | Hardware JSON `sensors[]` + `actuators[]` (heater/fan types & pins) |
| `display.*` | Hardware JSON `display` (driver, bus, geometry, layout.fields, refresh_rate_hz) |
| `filament_profiles[]` | Profiles NVS (builtins + customs; shadow rules per FR-008) |
| `device_model` / `flash_size_mb` | Flash target selection only (not runtime NVS) |
| `firmware_version` | Must equal bundled artifact / `GET /api/info.firmware_version` after boot |

## Actuator defaults from pin mapping

- Heater: type `mosfet_pwm` (or user-advanced override later), pin `heater_pwm`
- Fan: `fan_pwm` if `exhaust_fan_pwm` set; `fan_digital` if only digital pin set
- Control block: default `algorithm: "pid"` with safe `safety_limits.max_temp_c: 80` unless advanced UI sets otherwise

## Validation before write

- Reuse firmware pin rules: reject ESP32 input-only GPIOs 34–39 for outputs; detect duplicates; I2C pair completeness.
- Abort flash if validation fails (constitution Failsafe).

## Post-flash verify (optional but preferred)

1. Reset device.
2. Wait for USB serial or WiFi (if credentials valid).
3. Prefer: open WebSocket `ws://<ip>/ws` or hit `GET /api/info` when STA up; confirm `firmware_version`.
4. If WiFi fails: device should enter hotspot `philarmony` (firmware FR-001) — installer shows troubleshooting, not as hard failure if flash+verify of binary succeeded.
