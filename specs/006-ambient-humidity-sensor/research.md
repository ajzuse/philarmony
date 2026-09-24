# Research: Ambient Humidity Sensor Support

**Feature**: `006-ambient-humidity-sensor` | **Date**: 2026-09-03

## R1: Sensor Catalog & Reuse

**Decision**: Ambient sensor reuses existing `ISensorDriver` interface and `DriverRegistry` factory. No new driver code.

**Rationale**: All 8 sensor types (SHT31, SHT3x, BME280, AHT20, DHT22, DS18B20, NTC, Custom) already implement `ISensorDriver` with `begin(config)`, `read()` returning `SensorReading{temperature, humidity}`. The `DriverRegistry::createSensor(type, config)` factory already handles any registered type. Adding a second sensor instance is trivial.

**Alternatives considered**:
- New `IAmbientSensorDriver` interface → Rejected: duplicative; the physical sensor is identical, only the role differs.
- Ambient-only sensor subset → Rejected: limits hardware flexibility without benefit.

## R2: Ambient Sensor Configuration Shape

**Decision**: Add ambient fields to existing `SensorConfig` struct, mirroring chamber sensor fields.

```cpp
// New fields in SensorConfig
String ambient_type;                    // sensor type string (empty = not configured)
uint8_t ambient_i2c_address = 0;
int8_t ambient_gpio_pin = -1;
int8_t ambient_sda_pin = 21;
int8_t ambient_scl_pin = 22;
float ambient_temperature_offset = 0.0f;
float ambient_temperature_scale = 1.0f;
float ambient_humidity_offset = 0.0f;
float ambient_humidity_scale = 1.0f;
```

**Rationale**: Follows existing pattern for `humidity_type` (separate humidity sensor) and `extra_temp_type` (extra temperature sensor). NVS persistence via existing JSON serialization.

**Alternatives considered**:
- Separate `AmbientConfig` struct → Rejected: would require new NVS key, new getter/setter, new parser path. Existing pattern already supports multiple sensor roles in one struct.
- Inline JSON object in sensor config → Rejected: breaks flat struct pattern used for all other sensors.

## R3: HardwareConfigParser Ambient Role

**Decision**: Parser recognizes `"role": "ambient"` in sensor JSON objects.

**Rationale**: Existing parser uses `"capabilities": ["temperature", "humidity"]` to distinguish sensor roles. Adding `"role": "ambient"` is a clean extension that doesn't break existing config formats. Single-sensor configs without `"role"` default to chamber (backward compatible).

**Alternatives considered**:
- New capability `"ambient"` → Rejected: capabilities describe what a sensor measures, not where it's placed.
- Separate config section `"ambient_sensor"` → Rejected: breaks existing flat sensor array pattern.

## R4: Fan Stop Logic Architecture

**Decision**: Insert ambient-aware fan-stop check into `controlLoopTask()` after existing fan duty calculation.

**Logic**:
```
if (ambientSensor && ambientReading.valid &&
    !isnan(ambientHumidity) &&
    chamberHumidity <= ambientHumidity + uselessVentilationMargin) {
    fanPower = 0.0f;  // ventilation useless
}
```

**Rationale**: The fan control already lives in `controlLoopTask()` (main.cpp lines 419-423). The ambient check is a simple conditional that overrides fan duty. It runs at 50Hz like the rest of the control loop. The margin is read from `UISettings` (configurable in Settings, 0–10%RH, default 2%RH).

**Alternatives considered**:
- Extract fan decision into separate `FanController` class → Deferred: would be cleaner but is a refactor beyond scope of this feature. The existing inline pattern works and is consistent with how heater duty is calculated.
- Event-driven fan stop (interrupt on ambient change) → Rejected: adds complexity; 50Hz polling is sufficient for 30s timeout.

## R5: Ambient Sensor Timeout & Fallback

**Decision**: 30-second timeout; fallback to chamber-only fan logic.

**Implementation**: Track `lastValidAmbientMs` timestamp. If `millis() - lastValidAmbientMs > 30000`, treat ambient as unavailable. Fan logic falls through to existing chamber-only behavior.

**Rationale**: 30s matches typical I2C sensor polling intervals and is fast enough for SC-004 (fallback within 30s) while tolerant of transient bus noise.

**Alternatives considered**:
- Configurable timeout → Deferred: 30s is reasonable default; configurability adds UI surface for marginal benefit.
- No timeout (immediate fallback) → Rejected: would cause fan flickering on brief I2C glitches.

## R6: Useless-Ventilation Margin Configuration

**Decision**: Configurable in Settings, range 0–10%RH, default 2%RH. Stored in `UISettings`.

**Rationale**: Clarify session 2026-09-03 confirmed configurability. Range 0–10% covers reasonable use cases (0 = strict ambient equality, 10 = aggressive fan stopping). Default 2% is physically sensible.

**Storage**: Add `ambient_ventilation_margin_pct` to `UISettings` struct. Persist via existing NVS mechanism.

## R7: WebSocket Status Extension

**Decision**: Add `ambient_temp_c`, `ambient_humidity_pct`, `ambient_sensor_type` to `status/update` payload.

**Rationale**: FR-006 requires ambient on both touch UI and WebSocket. Fields follow existing naming conventions (`chamber_temp_c`, `humidity_pct`). Omitted when ambient not configured.

**Alternatives considered**:
- Nested `"ambient": {...}` object → Rejected: flat fields are consistent with existing payload structure and easier for clients to parse.

## R8: Touch UI Ambient Display

**Decision**: Show ambient readings on monitoring screen (below chamber values) and settings sensors screen (config display).

**Rationale**: Monitoring is the primary operational screen. Settings sensors already shows sensor config. Home screen is space-constrained; ambient on home is optional polish.

**Alternatives considered**:
- New dedicated ambient detail screen → Rejected: over-engineering for two extra labels.
- Ambient-only on WebSocket (no touch UI) → Rejected: FR-006 requires both.

## R9: DisplayManager Layout Extension

**Decision**: Add `"ambient_temp_c"` and `"ambient_humidity_pct"` to `all_fields` in `autoConfigureLayout()`.

**Rationale**: Larger displays (ILI9341 320×240, ILI9488) have room for ambient fields. Smaller displays (ST7735 128×160) skip them via existing pixel-count thresholds.

## R10: Test Strategy

**Decision**: Three test suites, all native/host:

1. **test_ambient_config_parser**: Validate `"role": "ambient"` parsing, pin conflict detection, invalid type rejection.
2. **test_ambient_nvs_roundtrip**: Validate ambient fields persist/load via ConfigManager NVS.
3. **test_ambient_fan_stop**: Extract fan-stop decision into testable function; validate margin comparison, timeout fallback, chamber-only backward compatibility.

**Rationale**: Constitution requires automated flow tests on product specs. Fan logic must be testable without hardware. Extracting the decision function is minimal refactoring.

**Alternatives considered**:
- Hardware-in-the-loop tests → Deferred to `005-manual-validation`.
- Mock sensor drivers → Not needed; native tests use existing `UNIT_TEST` guards.
