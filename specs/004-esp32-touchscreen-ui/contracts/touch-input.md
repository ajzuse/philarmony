# Contract: Touch Input

**Feature**: `004-esp32-touchscreen-ui`  
**Consumers**: `TouchManager`, LVGL indev port, settings calibration wizard

## ITouchDriver

```cpp
struct TouchPoint {
    bool pressed = false;
    int16_t x = 0;
    int16_t y = 0;
    uint32_t timestamp_ms = 0;
};

class ITouchDriver {
public:
    virtual ~ITouchDriver() = default;
    virtual bool begin(const JsonObject& config) = 0;
    virtual TouchPoint read() = 0;
    virtual bool isConnected() = 0;
    virtual String getType() const = 0;
    virtual void setSensitivity(const String& level) = 0; // low|medium|high
};
```

## Probe order (`auto`)

1. I2C scan known addresses: FT6236 (`0x38`), GT911 (`0x5D`/`0x14`), CST816S (`0x15`), STMPE610 I2C
2. SPI CS candidates from `TouchConfig` / board profile (CYD XPT2046)
3. If none → `controller_type=none`; UI disabled, legacy display path only

## Event filtering

| Parameter | Value |
|-----------|-------|
| Press debounce | 50ms |
| Release debounce | 100ms |
| Drag threshold | 8px |
| Coordinate space | After calibration, display logical WxH |

## Calibration

- 4-point target wizard
- Persist to NVS under TouchConfig.calibration
- Invalid/missing calibration → force wizard before accepting Start (P1 may use factory defaults for known boards)

## LVGL indev

- Type: `POINTER`
- `read_cb` pulls latest filtered `TouchPoint`
- No multi-touch
