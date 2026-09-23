# GPIO Connections — Philarmony

Comprehensive diagram of connections supported by the firmware on the 3 boards.

## 1. General diagram

```
                          ┌─────────────────────────────────────────────┐
                          │              ESP32 (3 models)               │
                          │                                             │
                          │  ┌───────────┐  ┌──────────┐  ┌──────────┐  │
          I2C SDA=21 ─────┼─►│ SHT31     │  │ Heater   │  │ Fan      │  │
          I2C SCL=22 ─────┼─►│ BME280    │  │ MOSFET   │  │ PWM/DIG  │  │
   (chamber sensors +     │  │ AHT20     │  │ PWM=25   │  │ PWM=26   │  │
    optional ambient)     │  │ SHT3x     │  │ 1 kHz    │  │ 5 kHz    │  │
                          │  └───────────┘  └──────────┘  └──────────┘  │
                          │                                             │
   1-Wire GPIO 4 ─────────┼─► DHT22 / DS18B20   (alternate sensors)    │
   ADC GPIO 34 ───────────┼─► NTC / Thermistor                           │
                          │                                             │
                          │  ┌──────────────────────────────────────┐   │
   SPI (display)  ────────┼─►│ Onboard or external display          │   │
                          │  │ ST7789 / ILI9341 / ILI9488 / GC9A01  │   │
                          │  │ SSD1306 / SH1106 (I2C OLED)          │   │
                          │  │ HD44780 (LCD 16x2) / Nextion (UART)  │   │
                          │  └──────────────────────────────────────┘   │
                          │                                             │
   Touch (SPI/I2C) ───────┼─► XPT2046 / FT6236 / GT911 / CST816S        │
                          │                                             │
   Custom GPIO ───────────┼─► Haptic/buzzer (optional)                  │
                          │                                             │
                          └─────────────────────────────────────────────┘
```

## 2. Pin map per board (factory defaults)

```
┌──────────────────────┬────────────┬───────────────────┬──────────────────┐
│ Signal               │ WROOM-32U  │ LILYGO T-Display  │ ESP32-2432S028   │
│                      │ (esp32dev) │ (ttgo-t-display)  │ (CYD)            │
├──────────────────────┼────────────┼───────────────────┼──────────────────┤
│ I2C SDA              │ GPIO 21    │ GPIO 21           │ GPIO 21 ⚠        │
│ I2C SCL              │ GPIO 22    │ GPIO 22           │ GPIO 22          │
│ Heater PWM           │ GPIO 25    │ GPIO 25           │ GPIO 25          │
│ Fan PWM              │ GPIO 26    │ GPIO 26           │ GPIO 26          │
│ 1-Wire (DHT22/DS18B) │ GPIO 4     │ GPIO 4            │ GPIO 4           │
│ NTC ADC              │ GPIO 34    │ GPIO 34           │ GPIO 34          │
│ Haptic/buzzer        │ -1 (disab) │ -1 (disab)        │ -1 (disab)       │
├──────────────────────┼────────────┼───────────────────┼──────────────────┤
│ Display              │ external   │ ST7789 onboard    │ ILI9341 onboard  │
│   SPI MOSI           │ (varies)   │ GPIO 19           │ GPIO 13          │
│   SPI SCLK           │            │ GPIO 18           │ GPIO 14          │
│   SPI CS             │            │ GPIO 5            │ GPIO 15          │
│   DC                 │            │ GPIO 16           │ GPIO 2           │
│   RST                │            │ GPIO 23           │ -1 (hardwired)   │
│   Backlight          │            │ GPIO 4 ⚠          │ GPIO 21 ⚠⚠       │
│   Resolution         │            │ 135×240           │ 240×320          │
├──────────────────────┼────────────┼───────────────────┼──────────────────┤
│ Touch                │ external   │ none onboard      │ XPT2046 resist.  │
│   SPI CS             │ (varies)   │ -1                │ not committed*   │
│   SPI MOSI           │            │ -1                │ not committed*   │
│   SPI MISO           │            │ -1                │ not committed*   │
│   SPI SCLK           │            │ -1                │ not committed*   │
│   IRQ                │            │ -1                │ not committed*   │
├──────────────────────┼────────────┼───────────────────┼──────────────────┤
│ Touch I2C (auto)     │ FT6236/GT9 │ CST816S etc.      │ (via I2C if cfg) │
│   (same bus          │ addr 0x38  │ addr 0x15         │                  │
│    SDA/SCL)          │            │                   │                  │
└──────────────────────┴────────────┴───────────────────┴──────────────────┘

⚠  GPIO 21 shared: I2C SDA vs. ILI9341 backlight (CYD) — conflict!
⚠⚠ ILI9341 backlight=21 collides with default I2C SDA=21 on CYD.
*  XPT2046 (CYD touch) has no default pins in code — requires config JSON.
```

## 3. Variant A — WITHOUT ambient sensor (minimal config)

```
ESP32 (any board)
│
├── I2C (SDA=21, SCL=22)
│   └── Chamber sensor: SHT31 @0x44  (or BME280, AHT20, SHT3x)
│
├── GPIO 25 ──► MOSFET (AOD4184) ──► Heater 12V/24V
│                PWM 1 kHz, 0-100% duty
│
├── GPIO 26 ──► Exhaust fan
│   ├── [WITH rotation]   MOSFET/fan_pwm @ 5 kHz — continuous 0-100% duty
│   └── [WITHOUT rotation] fan_digital — ON/OFF only
│
├── Display (per board)
│   └── SPI or I2C per table above
│
└── Touch (optional, per board)

Nothing connected on: GPIO 4, GPIO 34, custom GPIO (-1)
WS telemetry: chamber_temp_c, humidity_pct (no ambient_*)
```

## 4. Variant B — WITH ambient sensor

```
ESP32 (any board)
│
├── I2C (SDA=21, SCL=22)
│   ├── Chamber sensor:   SHT31 @0x44  (role "chamber")
│   └── Ambient sensor:   DHT22 @GPIO 4 OR another I2C @different address
│                         (role: "ambient" — may be a DIFFERENT model)
│
├── GPIO 25 ──► MOSFET ──► Heater
│
├── GPIO 26 ──► Exhaust fan
│   ├── [WITH rotation]   0-100% duty + temp→power curve
│   └── [WITHOUT rotation] on/off
│
├── Ambient-aware logic (firmware):
│   IF chamber_RH ≤ ambient_RH + margin (0-10%, default 2%)
│   THEN fan = 0 (ventilation useless)
│   ELSE fan = normal duty
│   Fallback: ambient invalid >30s → chamber only
│
├── Display + Touch (per board)
│
└── Additional WS telemetry:
    ambient_temp_c, ambient_humidity_pct, ambient_sensor_type
    Touch UI: "Amb: 24.1°C / 55.2%" on monitoring screen
```

## 5. Fan: 3 rotation control modes

```
┌─────────────────────────────────────────────────────────────┐
│                    fan_type / fan_mode                      │
├──────────────────┬──────────────────┬───────────────────────┤
│  fan_pwm         │  fan_digital     │  shared_mosfet        │
│  independent_pwm │  independent_    │  shared_mosfet        │
│  (default)       │  digital         │                       │
├──────────────────┼──────────────────┼───────────────────────┤
│  LEDC 10-bit     │  digitalWrite    │  No dedicated pin     │
│  5 kHz           │  HIGH/LOW        │  Heater+fan on the    │
│                  │                  │  SAME pin             │
│  Continuous      │  0% = OFF        │  Split 80%/20%        │
│  0-100% duty     │  >0% = ON        │  (heater/fan)         │
├──────────────────┼──────────────────┼───────────────────────┤
│  Temp curve:     │  Curve ignored   │  Curve ignored        │
│  yes             │  (on/off only)   │                       │
├──────────────────┼──────────────────┼───────────────────────┤
│  GPIO 26         │  GPIO 26         │  GPIO 25 (heater)     │
│  dedicated MOSFET│  relay/transistor│  single MOSFET        │
└──────────────────┴──────────────────┴───────────────────────┘

Example curve (fan_pwm):
  40°C → 50%   ─┐
  60°C → 100%  ─┘  linear interpolation
```

## 6. Full support matrix

```
┌─────────────────┬────────────────────────────────────────────┐
│ Category        │ Supported types                            │
├─────────────────┼────────────────────────────────────────────┤
│ Sensors (15)    │ sht3x, sht30, sht31, dht22, dht11,        │
│                 │ am2302, ds18b20, ds18s20, ntc, thermistor,│
│                 │ bme280, bmp280, aht20, aht10, custom      │
│                 │ [registered: 9 of 15]                      │
├─────────────────┼────────────────────────────────────────────┤
│ Sensor roles    │ chamber, ambient,                          │
│                 │ humidity (separate), extra_temp            │
├─────────────────┼────────────────────────────────────────────┤
│ Actuators (3)   │ heater — REQUIRED                          │
│                 │ fan (exhaust) — optional                   │
│                 │ custom (buzzer/haptic) — optional          │
├─────────────────┼────────────────────────────────────────────┤
│ Actuator types  │ mosfet_pwm, mosfet_aod4184, ssr,           │
│                 │ fan_pwm, fan_digital, shared_mosfet,       │
│                 │ stepper, servo, gpio, custom               │
│                 │ [triac REJECTED]                           │
├─────────────────┼────────────────────────────────────────────┤
│ Display (9+2)   │ st7789, ili9341, st7735, gc9a01, ili9488,  │
│                 │ ssd1306, sh1106, hd44780, nextion,        │
│                 │ auto (auto-probe), none                    │
├─────────────────┼────────────────────────────────────────────┤
│ Touch (5)       │ resistive:  xpt2046, stmpe610              │
│                 │ capacitive: ft6236, gt911, cst816s         │
│                 │ auto (I2C first, then SPI), none           │
├─────────────────┼────────────────────────────────────────────┤
│ Control (3)     │ pid, bang_bang, pwm_feedforward            │
│                 │ [custom REJECTED until plugin]             │
├─────────────────┼────────────────────────────────────────────┤
│ Buses (5)       │ i2c, spi, onewire, uart, adc               │
│                 │ [parallel_8bit REJECTED]                   │
├─────────────────┼────────────────────────────────────────────┤
│ Boards (3)      │ ESP32WROOM-32U (esp32dev)                  │
│                 │ LILYGO T-Display (ttgo-t-display)          │
│                 │ ESP32-2432S028 CYD (esp32_2432s028)        │
└─────────────────┴────────────────────────────────────────────┘
```

## Critical notes

- **No separate cooling fan** — the only fan is the exhaust, which also runs during COOLDOWN (30s post-cycle).
- **CYD conflict**: ILI9341 backlight=GPIO21 collides with I2C SDA=GPIO21 — one of them must be remapped.
- **CYD touch**: XPT2046 has no default pins in code — requires explicit config JSON.
- All 3 boards share the same I2C/heater/fan pins — the difference is only the onboard display and touch.
