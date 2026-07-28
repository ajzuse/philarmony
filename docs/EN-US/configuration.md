# Philarmony — Configuration (EN-US)

## WiFi

Stored in NVS via captive portal or `POST /api/wifi/config`. Missing/invalid credentials → AP within SC-01 fail-fast window.

## Hardware (`config/hardware`)

```json
{
  "sensors": [{"id":"chamber_temp","type":"sht31","capabilities":["temperature","humidity"],
    "bus":{"type":"i2c","sda_pin":21,"scl_pin":22,"address":68},
    "calibration":{"temperature_offset":0,"humidity_scale":1}}],
  "actuators": [{"id":"heater","type":"mosfet_pwm","role":"heater","pins":{"pwm":25},
    "control":{"pwm_freq_hz":1000,"max_power_pct":100},
    "safety_limits":{"max_temp_c":80,"max_power_pct":100}},
    {"id":"exhaust_fan","type":"fan_pwm","role":"fan","pins":{"pwm":26},
      "control":{"cooldown_sec":30,"speed_curve":[{"temp_c":40,"power_pct":50},{"temp_c":60,"power_pct":100}]}}],
  "display": {"enabled":true,"driver":"st7789",
    "bus":{"type":"spi","mosi":19,"sclk":18,"cs":5,"dc":16,"rst":23,"bl":4},
    "geometry":{"width":135,"height":240,"rotation":90},
    "layout":{"fields":["chamber_temp_c","status"],"font_scaling":"auto","compact_mode":false}},
  "control": {"algorithm":"pid","parameters":{"kp":12.5,"ki":0.45,"kd":32.1},
    "safety_limits":{"hard_temp_limit_c":80,"sensor_timeout_ms":600,"max_heater_power_pct":100}}
}
```

### Rules

- Actuator/display output pins: GPIO **0–33** (34–39 rejected).
- `algorithm: "custom"` rejected until a plugin registers a factory.
- `triac` / `parallel_8bit` rejected (not shipped).
- Optional heater `current_sense_pin` + `overcurrent_adc_threshold` for ACTUATOR_FAULT feedback.

## Profiles

Built-ins: pla, petg, abs, tpu, nylon. Up to 20 customs. Ranges: temp 30–80 °C, duration 1–1440 min, humidity 5–50%.

## Build / flash (Makefile)

Preferred host commands (see `specs/001-filament-dryer-esp32/contracts/makefile-targets.md`):

```bash
make build                 # ENV=esp32devkitc by default
make test                  # native Unity suite
make flash PORT=/dev/ttyUSB0   # requires attached ESP32; aliases: upload, install
make help
```
