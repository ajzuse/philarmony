# Quickstart Validation: Ambient Humidity Sensor Support

**Feature**: `006-ambient-humidity-sensor` | **Date**: 2026-09-03

## Prerequisites

- ESP32 with chamber sensor (SHT31 or equivalent) configured and working
- Second sensor (any supported type: SHT31, DHT22, BME280, AHT20) wired to available I2C/SPI pins
- Firmware built with ambient sensor support
- Serial monitor or WebSocket client for status observation

## QS-1: Chamber-Only Backward Compatibility

**Proves**: Single-sensor builds unaffected (US2)

```bash
# Setup: No ambient sensor configured
# 1. Build firmware
make build ENV=esp32_2432s028

# 2. Flash and boot
# 3. Start a drying cycle via touch UI or WebSocket
# 4. Observe: fan behavior matches pre-feature behavior
# 5. Verify status/update has NO ambient_temp_c / ambient_humidity_pct fields
```

**Expected**: Drying cycle runs identically to pre-006 firmware. No ambient fields in status.

## QS-2: Ambient Sensor Fan Stop

**Proves**: Fan stops when chamber humidity approaches ambient (US1)

```bash
# Setup: Configure ambient sensor in hardware config JSON
# {
#   "sensors": [
#     {"type": "sht31", "capabilities": ["temperature", "humidity"], "i2c_address": "0x44"},
#     {"type": "dht22", "role": "ambient", "capabilities": ["temperature", "humidity"], "gpio_pin": 4}
#   ]
# }

# 1. Build and flash
# 2. Place ambient sensor in room air (not inside chamber)
# 3. Start drying cycle with target humidity below ambient
#    (e.g., target 15%RH, ambient ~55%RH, chamber starts ~40%RH)
# 4. Observe: fan runs while chamber humidity >> ambient + margin
# 5. When chamber humidity drops to within 2%RH of ambient:
#    Fan STOPS (ventilation useless)
# 6. Verify: chamber humidity stabilizes near ambient (can't dry below ambient)
```

**Expected**: Fan stops when chamber humidity ≤ ambient + 2%RH. SC-001: stop within 5s.

## QS-3: Configurable Margin

**Proves**: Margin adjustable in Settings (FR-004)

```bash
# 1. With ambient sensor configured, open Settings on touch UI
# 2. Navigate to Sensors > Ambient Ventilation Margin
# 3. Change margin from 2%RH to 5%RH
# 4. Start a drying cycle
# 5. Fan should stop when chamber humidity is within 5%RH of ambient
#    (earlier than with 2%RH margin)
```

**Expected**: Fan stop point adjusts with margin setting.

## QS-4: Ambient Fallback on Sensor Loss

**Proves**: 30s timeout fallback (US4, SC-004)

```bash
# 1. Start dual-sensor drying cycle
# 2. While fan is running, disconnect ambient sensor I2C/SPI wire
# 3. Observe: after ≤30s, fan resumes normal duty (chamber-only logic)
# 4. Touch UI shows ambient as "--" / "N/A"
# 5. WebSocket status shows ambient_temp_c: null
# 6. No device fault raised
# 7. Reconnect ambient sensor wire
# 8. Observe: within next read cycle, ambient-aware logic resumes
```

**Expected**: Fallback within 30s. No fault. Recovery on reconnect.

## QS-5: WebSocket Ambient Status

**Proves**: Ambient in status/update (FR-006)

```bash
# 1. Connect WebSocket client to device
# 2. Subscribe to status/update
# 3. With ambient configured, observe payload includes:
#    - ambient_temp_c
#    - ambient_humidity_pct
#    - ambient_sensor_type
# 4. Without ambient, observe these fields are absent
```

**Expected**: Fields present when configured, absent when not.

## QS-6: Mixed Sensor Types

**Proves**: Chamber and ambient can use different models (FR-001a)

```bash
# Config: Chamber = BME280, Ambient = DHT22
# {
#   "sensors": [
#     {"type": "bme280", "capabilities": ["temperature", "humidity", "pressure"]},
#     {"type": "dht22", "role": "ambient", "capabilities": ["temperature", "humidity"], "gpio_pin": 4}
#   ]
# }

# 1. Build, flash, start cycle
# 2. Both sensors read independently
# 3. Ambient-aware fan logic uses DHT22 ambient reading
# 4. Status shows both sensor types
```

**Expected**: Mixed types work. Fan logic uses ambient regardless of type mismatch.
