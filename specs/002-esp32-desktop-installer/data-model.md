# Data Model: ESP32 Desktop Installer

**Feature**: `002-esp32-desktop-installer` | **Date**: 2026-07-28  
**Aligned with**: `specs/001-filament-dryer-esp32/data-model.md` (device-side entities)

## Entities

### InstallerSession
In-memory wizard state (not persisted until profile export or flash).

| Field | Type | Notes |
|-------|------|-------|
| `session_id` | UUID | Generated on app launch / New Setup |
| `step` | enum | `device` → `sensors` → `pins` → `display` → `profiles` → `wifi` → `review` → `flash` |
| `locale` | `pt-BR` \| `en-US` | UI language |
| `device_profile` | DeviceProfile | Accumulated config |
| `validation_errors` | string[] | Blocking errors for current step |
| `flash_job` | FlashJob \| null | Active/last flash |

### DeviceProfile
Exportable configuration (see `contracts/installer-profile.schema.json`).

| Field | Type | Validation |
|-------|------|------------|
| `profile_version` | number | Currently `1` |
| `device_model` | enum | `ESP32`, `ESP32-S2`, `ESP32-S3`, `ESP32-C3` |
| `flash_size_mb` | 4 \| 8 \| 16 | From detect or manual |
| `sensors` | SensorConfig[] | ≥0; pin conflicts checked |
| `pin_mapping` | PinMapping | `heater_pwm` required |
| `display` | DisplayConfig | |
| `filament_profiles` | FilamentProfile[] | ≤20 customs; builtins read-only |
| `wifi` | WiFiConfig | SSID required to flash |
| `firmware_version` | string | Must match bundled artifact |
| `created_at` / `updated_at` | ISO-8601 | Profile metadata |

### SensorConfig

| Field | Type | Validation |
|-------|------|------------|
| `role` | `temperature` \| `humidity` \| `pressure` \| `extra` | |
| `sensor` | string | Driver id: `dht22`, `ds18b20`, `bme280`, `sht3x`, `ntc`, `aht20`, `none` |
| `gpio_pin` | number \| null | Required unless `none` / I2C-only |
| `i2c_address` | number \| null | When bus is I2C |
| `parameters` | object | Optional calibration/resolution |

### PinMapping

| Field | Type | Validation |
|-------|------|------------|
| `heater_pwm` | number | Required; output-capable (not 34–39 on classic ESP32) |
| `exhaust_fan_pwm` | number \| null | Mutually exclusive with digital/shared rules |
| `exhaust_fan_digital` | number \| null | |
| `i2c_sda` / `i2c_scl` | number \| null | Pair required if I2C used |
| `spi_*` | number \| null | Required when SPI display selected |

**Rules**: no duplicate pins; reserved/strapping pins warned or blocked; PWM functions only on PWM-capable pins (model-specific table).

### DisplayConfig

| Field | Type | Validation |
|-------|------|------------|
| `enabled` | boolean | |
| `driver` | enum | `ssd1306`, `sh1106`, `st7789`, `ili9341`, `auto`, … (subset of firmware) |
| `width` / `height` | number | Preset or custom |
| `i2c_address` | number \| null | |
| `fields` | string[] | Subset of status payload keys |
| `refresh_rate_hz` | 1–5 | Default 1 |

### WiFiConfig

| Field | Type | Validation |
|-------|------|------------|
| `ssid` | string | Non-empty for flash |
| `password` | string | In-session only; export as `***` / omit |
| `static_ip` | object \| null | Optional ip/gateway/netmask/dns |

### FilamentProfile
Same semantic as firmware `001` (id, names PT/EN, temp 30–80, duration 1–1440, humidity 5–50, `is_builtin`).

### FirmwarePackage

| Field | Type | Notes |
|-------|------|-------|
| `version` | semver string | |
| `app_binary` | path | Bundled |
| `bootloader` / `partition_table` | path \| null | As required by flash path |
| `supported_chips` | string[] | |
| `checksum_sha256` | string | Verified before flash |

### HostReleaseArtifact
Build/release metadata for the **desktop app installer** (not the ESP32 image).

| Field | Type | Notes |
|-------|------|-------|
| `app_version` | semver | Philarmony Desktop Installer version |
| `os` | `windows` \| `macos` \| `linux` | |
| `arch` | `x64` \| `arm64` \| `universal` | |
| `format` | `msix` \| `dmg` \| `appimage` \| `deb` \| `rpm` \| `exe` | Win: msix; macOS: dmg; Linux: appimage+deb+rpm |
| `download_url` | string | GitHub Release asset |
| `checksum_sha256` | string | Published alongside asset |
| `signed` | boolean | Code-signed / notarized |
| `embeds_firmware_version` | string | Must match bundled `FirmwarePackage.version` |

### FlashJob

| Field | Type | Notes |
|-------|------|-------|
| `state` | enum | `idle`, `erasing`, `writing_app`, `writing_nvs`, `verifying`, `resetting`, `success`, `failed` |
| `progress_pct` | 0–100 | |
| `log_lines` | string[] | FR-011 diagnostics |
| `error` | string \| null | |
| `network_verify` | enum | `pending`, `ok`, `skipped`, `warn` — soft; never flips `success`→`failed` |
| `started_at` / `finished_at` | ISO-8601 | |

## Relationships

```text
InstallerSession 1──1 DeviceProfile
DeviceProfile 1──* SensorConfig
DeviceProfile 1──1 PinMapping
DeviceProfile 1──1 DisplayConfig
DeviceProfile 1──1 WiFiConfig
DeviceProfile 1──* FilamentProfile
InstallerSession 0..1──1 FlashJob
FlashJob *──1 FirmwarePackage
HostReleaseArtifact *──1 FirmwarePackage   # embeds firmware version
```

## State transitions

### Wizard step machine
`device` → `sensors` → `pins` → `display` → `profiles` → `wifi` → `review` → `flash`  
Back allowed except during active flash. Next blocked while `validation_errors` non-empty.

### FlashJob
`idle` → `erasing` → `writing_app` → `writing_nvs` → `verifying` → `resetting` → `success`  
Any stage → `failed` (**Retry** = restart full pipeline with same package; **no** auto-restore of prior image).  
After `success`, optional soft `network_verify` → `ok` \| `skipped` \| `warn` (never fails the job).

## Mapping to firmware (001)

Installer `DeviceProfile` transforms into firmware boot config:
- WiFi → NVS wifi keys
- sensors + pin_mapping + display → hardware JSON (`sensors[]`, `actuators[]`, `display`, `control`)
- filament_profiles → profiles NVS
- See `contracts/nvs-config-mapping.md`
