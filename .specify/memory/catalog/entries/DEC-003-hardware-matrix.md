# DEC-003: Hardware Support Matrix

**Tags:** hardware, drivers  
**Source:** research.md §3–4  
**Updated:** 2026-07-28

## Sensors
sht3x/sht30/sht31, aht20, bme280, dht22, ds18b20, ntc, custom.

## Actuators
heater: mosfet_pwm, ssr, custom. fan: fan_pwm, fan_digital, shared_mosfet, custom.
Also supported: stepper, servo, gpio.

**Rejected at parse (no driver):** `triac` — research listed it but firmware rejects with a clear error until a factory is registered.

## Displays
ssd1306, sh1106, st7789, ili9341, st7735, gc9a01, ili9488, hd44780, nextion, auto.
Bus types: i2c, spi, uart (and onewire/adc for sensors).

**Rejected at parse (no driver):** `parallel_8bit` — listed in early schema drafts but not shipped; parser rejects unknown/unsupported bus.

## Control algorithms
pid, bang_bang, pwm_feedforward.
**Rejected at parse:** `custom` until a plugin registers a factory (`custom algorithm requires registered factory`).

## Test boards
ESP32_DEVKITC_V4, LilyGo T-Display V1.1, ESP32-2432S028 (CYD).
