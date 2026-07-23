# ESP32 Desktop Installer - Base Configuration Tool

## Feature Overview

**Feature Name**: ESP32 Desktop Installer  
**Short Name**: esp32-desktop-installer  
**Version**: 0.1.0  
**Status**: Draft  
**Date**: 2026-07-23  

## Executive Summary

A desktop application that simplifies initial configuration and firmware installation for the Philarmony filament dryer ESP32 device. The installer provides a guided setup wizard allowing users to configure sensor types, ESP32 model, display presence/resolution, GPIO pin mappings, and WiFi credentials before flashing a pre-configured firmware binary to the device via USB. This eliminates the need for manual WebSocket configuration after first boot.

## User Scenarios & Testing

### Primary User Scenarios

**Scenario 1: First-Time Device Setup**
1. User connects ESP32 to computer via USB
2. Launches desktop installer application
3. Selects ESP32 model (WROOM-32, S3, etc.)
4. Configures sensor types (temperature: DHT22/DS18B20, humidity: DHT22)
5. Maps GPIO pins for heater PWM, exhaust fan PWM/digital, sensor data lines
6. Chooses display option (none, SSD1306 128x64, ST7789 240x240, etc.)
7. Enters WiFi credentials (SSID/password)
8. Clicks "Install" - application flashes pre-configured firmware
9. Device reboots, connects to WiFi, starts WebSocket server ready for use

**Scenario 2: Reconfiguration of Existing Device**
1. User connects previously configured ESP32 via USB
2. Launches installer, detects existing configuration
3. Modifies sensor type, pin mapping, or WiFi credentials
4. Re-flashes updated firmware
5. Device retains new configuration on boot

**Scenario 3: Bulk Configuration for Multiple Devices**
1. User saves configuration profile as JSON file
2. Loads profile for subsequent devices
3. Rapidly flashes multiple ESP32s with identical settings

### Acceptance Criteria

| Scenario | Given | When | Then |
|----------|-------|------|------|
| First-time setup | ESP32 connected via USB, no firmware | User completes wizard and clicks Install | Firmware flashed with all settings embedded, device boots to WiFi-connected state |
| Sensor config | In sensor selection step | User selects DHT22 on GPIO4 | Firmware includes DHT22 driver initialized on GPIO4 |
| Pin mapping | In pin configuration step | User assigns heater to GPIO25, fan to GPIO26 | Firmware uses specified GPIOs for PWM output |
| Display config | In display step | User selects SSD1306 128x64, enables status fields | Firmware initializes I2C display with selected layout |
| WiFi config | In WiFi step | User enters SSID/password | Firmware stores credentials in NVS, connects on boot |
| Profile save/load | User has completed configuration | User clicks Save Profile | JSON file created with all settings for reuse |
| Flash verification | Flash completes | Device reboots | Installer verifies device responds on WebSocket port |

### Edge Cases

- USB device not detected: Clear error message with troubleshooting steps
- Invalid GPIO pin combination (conflicts): Validation prevents invalid mappings
- Flash failure: Rollback option, detailed error log, retry button
- Unsupported ESP32 model: Warning with list of supported variants
- WiFi credentials invalid: Device falls back to hotspot mode (per firmware spec)
- Display not detected on boot: Firmware continues headless, logs warning

## Functional Requirements

### FR-001: Device Detection & Connection
- Detect ESP32 devices connected via USB (USB-UART bridge: CP210x, CH340, FTDI)
- Identify chip model (ESP32, ESP32-S2, ESP32-S3, ESP32-C3) and flash size
- Establish serial communication for flashing at 921600 baud (configurable)

### FR-002: Guided Configuration Wizard
- Multi-step wizard: Device Model → Sensors → Pin Mapping → Display → WiFi → Review → Flash
- Each step validates input before allowing progression
- Visual indicators for required vs optional fields
- Back/Next navigation preserving entered data

### FR-003: Sensor Type Configuration
- Temperature sensor options: DHT22, DS18B20, BME280, "None"
- Humidity sensor options: DHT22, BME280, "None" (linked to temp sensor where applicable)
- Per-sensor GPIO pin assignment with conflict detection
- Custom sensor support via JSON driver specification (advanced)

### FR-004: GPIO Pin Mapping
- Visual pinout diagram for selected ESP32 model
- Assignable functions: Heater PWM, Exhaust Fan PWM, Exhaust Fan Digital, Temp Sensor, Humidity Sensor, I2C SDA/SCL, SPI MOSI/MISO/SCK/CS
- Validation: No duplicate pin assignments, PWM-capable pins for PWM functions, ADC pins for analog sensors
- Reserved pins highlighted (strapping pins, flash, PSRAM)

### FR-005: Display Configuration
- Enable/disable display toggle
- Display driver selection: SSD1306 (I2C), SH1106 (I2C), ST7789 (SPI), ILI9341 (SPI), "Auto-detect"
- Resolution presets per driver (128x64, 128x32, 240x240, 320x240, custom)
- Field selection for status display: checkbox list from firmware status payload (temperature, humidity, heater%, fan%, status, time, CPU%, RAM)
- Layout preview showing field arrangement

### FR-006: Filament Drying Profiles Configuration
- View built-in profiles: PLA (50°C/4h/15%), PETG (65°C/4h/15%), ABS (80°C/2h/10%), TPU (45°C/4h/20%), Nylon (70°C/6h/10%)
- Create custom profiles: name (PT-BR/EN-US), target temperature (30-80°C), default duration (1-1440 min), target humidity (5-50%)
- Edit/delete custom profiles (built-in profiles read-only but can be overridden)
- Profiles synced to firmware NVS during flash
- Max 20 custom profiles
- Profile validation on input

### FR-007: WiFi Credentials
- SSID input (scan for nearby networks optional)
- Password input with show/hide toggle
- Optional: Static IP configuration (IP, gateway, netmask, DNS)
- Credentials embedded in firmware NVS partition at flash time

### FR-008: Firmware Generation & Flashing
- Base firmware binary bundled with installer (versioned)
- Configuration injected into dedicated NVS partition or appended to binary
- Flash process: Erase → Write firmware → Write config partition → Verify → Reset
- Progress bar with stage indicators (erasing, writing, verifying)
- Support for custom partition tables

### FR-009: Configuration Profiles
- Export complete configuration to JSON file
- Import configuration from JSON file
- Profile includes: device model, sensors, pins, display, WiFi (password encrypted or placeholder)
- Version field for forward compatibility

### FR-010: Multi-language Support
- UI in Portuguese (primary) and English
- Language selector in application settings
- All user-facing strings externalized

### FR-011: Logging & Diagnostics
- Real-time log window during flash process
- Detailed error messages with suggested fixes
- Log export for troubleshooting
- Verbose/quiet mode toggle

## Non-Functional Requirements

### Performance
- Device detection: <2 seconds
- Flash process (4MB firmware): <30 seconds total
- UI responsiveness: <100ms interaction latency

### Usability
- Wizard completable in <3 minutes for typical setup
- No command-line knowledge required
- Tooltips for technical terms (PWM, GPIO, NVS, etc.)
- Accessible color contrast, keyboard navigation

### Reliability
- Verify firmware checksum after flash
- Automatic rollback on verification failure
- Survive USB disconnect during configuration (not during flash)

### Compatibility
- Windows 10/11 (x64), macOS 12+ (Intel/Apple Silicon), Linux (Ubuntu 20.04+, AppImage/Flatpak)
- ESP32 Arduino core compatible firmware format
- ESP-IDF bootloader compatible

## Key Entities

### DeviceProfile
- `device_model`: enum (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
- `flash_size_mb`: number (4, 8, 16)
- `sensors`: SensorConfig[]
- `pin_mapping`: PinMapping
- `display`: DisplayConfig
- `wifi`: WiFiConfig
- `firmware_version`: string

### SensorConfig
- `type`: enum (temperature: dht22, ds18b20, bme280, none; humidity: dht22, bme280, none)
- `gpio_pin`: number
- `parameters`: object (resolution, pull-up, etc.)

### PinMapping
- `heater_pwm`: number (required)
- `exhaust_fan_pwm`: number (optional)
- `exhaust_fan_digital`: number (optional)
- `i2c_sda`: number (optional)
- `i2c_scl`: number (optional)
- `spi_mosi`: number (optional)
- `spi_miso`: number (optional)
- `spi_sck`: number (optional)
- `spi_cs`: number (optional)

### DisplayConfig
- `enabled`: boolean
- `driver`: enum (ssd1306, sh1106, st7789, ili9341, auto)
- `width`: number
- `height`: number
- `i2c_address`: number (optional, for I2C displays)
- `spi_dc`: number (optional, for SPI displays)
- `spi_reset`: number (optional)
- `fields`: string[] (subset of status payload keys)

### WiFiConfig
- `ssid`: string
- `password`: string (encrypted in profile)
- `static_ip`: object (optional: ip, gateway, netmask, dns)

### FilamentProfile
- `id`: string (builtin: pla, petg, abs, tpu, nylon; custom: user-defined UUID)
- `name_pt`: string (Portuguese display name)
- `name_en`: string (English display name)
- `target_temp_c`: number (30-80)
- `default_duration_min`: number (1-1440)
- `target_humidity_pct`: number (5-50)
- `is_builtin`: boolean (read-only, true for default profiles)

### FirmwarePackage
- `version`: string
- `binary_path`: string
- `partition_table`: string
- `bootloader_path`: string
- `supported_chips`: string[]
- `checksum`: string (SHA256)

## Success Criteria

| Metric | Target | Measurement |
|--------|--------|-------------|
| Setup completion rate | >95% | Users reaching "Flash Success" screen / users starting wizard |
| Average setup time | <3 minutes | Timer from wizard start to flash complete |
| Flash success rate | >99% | Verified boots / flash attempts |
| Profile reuse rate | >50% | Profile loads / new configurations |
| Support tickets for setup | <5% of installs | Ticket volume vs download count |
| Cross-platform parity | 100% feature parity | Feature matrix across Win/Mac/Linux |

## Assumptions

1. Firmware binary is pre-built and versioned alongside installer releases
2. ESP32 firmware uses NVS partition for configuration (not compiled-in constants)
3. User has USB drivers for their ESP32 board pre-installed (CH340, CP210x, FTDI)
4. Installer bundles esptool.py or equivalent flashing library
5. No OTA update capability in this phase (USB-only flashing)
6. WiFi password stored in profile as placeholder; user re-enters on import
7. Single firmware binary supports all configured sensor/display combinations via runtime config
8. Installer does not modify bootloader or partition table (uses firmware-provided defaults)

## Dependencies & Constraints

- **Firmware coupling**: Installer must match firmware's NVS schema version
- **USB drivers**: OS-level requirement outside installer control
- **Signed binaries**: macOS/Windows may require code signing for distribution
- **GPLv3 compliance**: Installer source must be open; firmware already GPLv3
- **Constitutional**: PT-BR/EN-US documentation, safety validation for pin assignments

## Out of Scope (This Phase)

- OTA/Network firmware updates
- Firmware compilation from source (pre-built binaries only)
- Advanced scripting/CLI mode
- Cloud profile sync
- Multi-device simultaneous flashing
- Custom firmware module selection
- Real-time sensor calibration wizard

## Appendix: Configuration Profile JSON Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Philarmony ESP32 Installer Profile",
  "version": "1.0",
  "device_model": "ESP32-S3",
  "flash_size_mb": 8,
  "sensors": [
    { "type": "temperature", "sensor": "dht22", "gpio_pin": 4 },
    { "type": "humidity", "sensor": "dht22", "gpio_pin": 4 }
  ],
  "pin_mapping": {
    "heater_pwm": 25,
    "exhaust_fan_pwm": 26,
    "exhaust_fan_digital": 27,
    "i2c_sda": 21,
    "i2c_scl": 22
  },
  "display": {
    "enabled": true,
    "driver": "ssd1306",
    "width": 128,
    "height": 64,
    "i2c_address": 0x3C,
    "fields": ["chamber_temp_c", "humidity_pct", "heater_power_pct", "status", "elapsed_time_sec"]
  },
  "filament_profiles": [
    { "id": "pla", "name_pt": "PLA", "name_en": "PLA", "target_temp_c": 50, "default_duration_min": 240, "target_humidity_pct": 15, "is_builtin": true },
    { "id": "petg", "name_pt": "PETG", "name_en": "PETG", "target_temp_c": 65, "default_duration_min": 240, "target_humidity_pct": 15, "is_builtin": true },
    { "id": "abs", "name_pt": "ABS", "name_en": "ABS", "target_temp_c": 80, "default_duration_min": 120, "target_humidity_pct": 10, "is_builtin": true },
    { "id": "tpu", "name_pt": "TPU", "name_en": "TPU", "target_temp_c": 45, "default_duration_min": 240, "target_humidity_pct": 20, "is_builtin": true },
    { "id": "nylon", "name_pt": "Nylon", "name_en": "Nylon", "target_temp_c": 70, "default_duration_min": 360, "target_humidity_pct": 10, "is_builtin": true }
  ],
  "wifi": {
    "ssid": "MyNetwork",
    "password": "***ENCRYPTED***",
    "static_ip": null
  },
  "firmware_version": "0.1.0",
  "profile_version": 1
}
```