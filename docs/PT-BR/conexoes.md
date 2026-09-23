# Conexões GPIO — Philarmony

Diagrama abrangente das conexões suportadas pelo firmware nos 3 boards.

## 1. Diagrama geral

```
                          ┌─────────────────────────────────────────────┐
                          │              ESP32 (3 modelos)              │
                          │                                             │
                          │  ┌───────────┐  ┌──────────┐  ┌──────────┐  │
          I2C SDA=21 ─────┼─►│ SHT31     │  │ Heater   │  │ Fan      │  │
          I2C SCL=22 ─────┼─►│ BME280    │  │ MOSFET   │  │ PWM/DIG  │  │
   (sensores câmara +     │  │ AHT20     │  │ PWM=25   │  │ PWM=26   │  │
    ambient opcional)     │  │ SHT3x     │  │ 1 kHz    │  │ 5 kHz    │  │
                          │  └───────────┘  └──────────┘  └──────────┘  │
                          │                                             │
   1-Wire GPIO 4 ─────────┼─► DHT22 / DS18B20   (sensores alternativos)│
   ADC GPIO 34 ───────────┼─► NTC / Thermistor                           │
                          │                                             │
                          │  ┌──────────────────────────────────────┐   │
   SPI (display)  ────────┼─►│ Display onboard ou externo           │   │
                          │  │ ST7789 / ILI9341 / ILI9488 / GC9A01  │   │
                          │  │ SSD1306 / SH1106 (I2C OLED)          │   │
                          │  │ HD44780 (LCD 16x2) / Nextion (UART)  │   │
                          │  └──────────────────────────────────────┘   │
                          │                                             │
   Touch (SPI/I2C) ───────┼─► XPT2046 / FT6236 / GT911 / CST816S        │
                          │                                             │
   GPIO custom ───────────┼─► Haptic/buzzer (opcional)                  │
                          │                                             │
                          └─────────────────────────────────────────────┘
```

## 2. Mapa de pinos por board (padrões de fábrica)

```
┌──────────────────────┬────────────┬───────────────────┬──────────────────┐
│ Sinal                │ WROOM-32U  │ LILYGO T-Display  │ ESP32-2432S028   │
│                      │ (esp32dev) │ (ttgo-t-display)  │ (CYD)            │
├──────────────────────┼────────────┼───────────────────┼──────────────────┤
│ I2C SDA              │ GPIO 21    │ GPIO 21           │ GPIO 21 ⚠        │
│ I2C SCL              │ GPIO 22    │ GPIO 22           │ GPIO 22          │
│ Heater PWM           │ GPIO 25    │ GPIO 25           │ GPIO 25          │
│ Fan PWM              │ GPIO 26    │ GPIO 26           │ GPIO 26          │
│ 1-Wire (DHT22/DS18B) │ GPIO 4     │ GPIO 4            │ GPIO 4           │
│ NTC ADC              │ GPIO 34    │ GPIO 34           │ GPIO 34          │
│ Haptic/buzzer        │ -1 (desab) │ -1 (desab)        │ -1 (desab)       │
├──────────────────────┼────────────┼───────────────────┼──────────────────┤
│ Display              │ externo    │ ST7789 onboard    │ ILI9341 onboard  │
│   SPI MOSI           │ (varia)    │ GPIO 19           │ GPIO 13          │
│   SPI SCLK           │            │ GPIO 18           │ GPIO 14          │
│   SPI CS             │            │ GPIO 5            │ GPIO 15          │
│   DC                 │            │ GPIO 16           │ GPIO 2           │
│   RST                │            │ GPIO 23           │ -1 (hardwired)   │
│   Backlight          │            │ GPIO 4 ⚠          │ GPIO 21 ⚠⚠       │
│   Resolução          │            │ 135×240           │ 240×320          │
├──────────────────────┼────────────┼───────────────────┼──────────────────┤
│ Touch                │ externo    │ nenhum onboard    │ XPT2046 resist.  │
│   SPI CS             │ (varia)    │ -1                │ não commitado*   │
│   SPI MOSI           │            │ -1                │ não commitado*   │
│   SPI MISO           │            │ -1                │ não commitado*   │
│   SPI SCLK           │            │ -1                │ não commitado*   │
│   IRQ                │            │ -1                │ não commitado*   │
├──────────────────────┼────────────┼───────────────────┼──────────────────┤
│ Touch I2C (auto)     │ FT6236/GT9 │ CST816S etc.      │ (via I2C se cfg) │
│   (mesmo barramento  │ addr 0x38  │ addr 0x15         │                  │
│    SDA/SCL)          │            │                   │                  │
└──────────────────────┴────────────┴───────────────────┴──────────────────┘

⚠  GPIO 21 compartilhado: I2C SDA vs. backlight ILI9341 (CYD) — conflito!
⚠⚠ ILI9341 backlight=21 colide com I2C SDA=21 padrão no CYD.
*  XPT2046 (touch CYD) não tem pinos default no código — precisa JSON de config.
```

## 3. Variante A — SEM sensor ambiente (configuração mínima)

```
ESP32 (qualquer board)
│
├── I2C (SDA=21, SCL=22)
│   └── Sensor câmara: SHT31 @0x44  (ou BME280, AHT20, SHT3x)
│
├── GPIO 25 ──► MOSFET (AOD4184) ──► Aquecedor 12V/24V
│                PWM 1 kHz, 0-100% duty
│
├── GPIO 26 ──► Fan exaustão
│   ├── [COM rotação]   MOSFET/fan_pwm @ 5 kHz — duty 0-100% contínuo
│   └── [SEM rotação]   fan_digital — apenas LIGA/DESLIGA (on/off)
│
├── Display (conforme board)
│   └── SPI ou I2C conforme tabela acima
│
└── Touch (opcional, conforme board)

Nada conectado em: GPIO 4, GPIO 34, GPIO custom (-1)
Telemetria WS: chamber_temp_c, humidity_pct (sem ambient_*)
```

## 4. Variante B — COM sensor ambiente

```
ESP32 (qualquer board)
│
├── I2C (SDA=21, SCL=22)
│   ├── Sensor câmara:   SHT31 @0x44  (papel "chamber")
│   └── Sensor ambiente: DHT22 @GPIO 4 OU outro I2C @endereço diferente
│                        (role: "ambient" — pode ser modelo DIFERENTE)
│
├── GPIO 25 ──► MOSFET ──► Aquecedor
│
├── GPIO 26 ──► Fan exaustão
│   ├── [COM rotação]   duty 0-100% + curva temp→potência
│   └── [SEM rotação]   on/off
│
├── Lógica ambient-aware (firmware):
│   IF câmara_RH ≤ ambiente_RH + margem (0-10%, default 2%)
│   THEN fan = 0 (ventilação inútil)
│   ELSE fan = duty normal
│   Fallback: se ambiente inválido >30s → câmara apenas
│
├── Display + Touch (conforme board)
│
└── Telemetria WS adicional:
    ambient_temp_c, ambient_humidity_pct, ambient_sensor_type
    Touch UI: "Amb: 24.1°C / 55.2%" na tela de monitoramento
```

## 5. Fan: 3 modos de controle de rotação

```
┌─────────────────────────────────────────────────────────────┐
│                    fan_type / fan_mode                      │
├──────────────────┬──────────────────┬───────────────────────┤
│  fan_pwm         │  fan_digital     │  shared_mosfet        │
│  independent_pwm │  independent_    │  shared_mosfet        │
│  (padrão)        │  digital         │                       │
├──────────────────┼──────────────────┼───────────────────────┤
│  LEDC 10-bit     │  digitalWrite    │  Sem pino próprio     │
│  5 kHz           │  HIGH/LOW        │  Aquecedor+fan no     │
│                  │                  │  MESMO pino           │
│  Duty 0-100%     │  0% = OFF        │  Split 80%/20%        │
│  contínuo        │  >0% = ON        │  (heater/fan)         │
├──────────────────┼──────────────────┼───────────────────────┤
│  Curva de temp:  │  Curva ignorada  │  Curva ignorada       │
│  sim             │  (só on/off)     │                       │
├──────────────────┼──────────────────┼───────────────────────┤
│  GPIO 26         │  GPIO 26         │  GPIO 25 (heater)     │
│  MOSFET dedicado │  Relé/transistor │  MOSFET único         │
└──────────────────┴──────────────────┴───────────────────────┘

Curva de exemplo (fan_pwm):
  40°C → 50%   ─┐
  60°C → 100%  ─┘  interpolação linear
```

## 6. Matriz de suporte completo

```
┌─────────────────┬────────────────────────────────────────────┐
│ Categoria       │ Tipos suportados                           │
├─────────────────┼────────────────────────────────────────────┤
│ Sensores (15)   │ sht3x, sht30, sht31, dht22, dht11,        │
│                 │ am2302, ds18b20, ds18s20, ntc, thermistor,│
│                 │ bme280, bmp280, aht20, aht10, custom      │
│                 │ [registrados: 9 de 15]                     │
├─────────────────┼────────────────────────────────────────────┤
│ Papéis sensor   │ chamber (câmara), ambient (ambiente),      │
│                 │ humidity (umidade separada),               │
│                 │ extra_temp (temp extra)                    │
├─────────────────┼────────────────────────────────────────────┤
│ Atuadores (3)   │ heater (aquecedor) — OBRIGATÓRIO           │
│                 │ fan (exaustão) — opcional                  │
│                 │ custom (buzzer/haptic) — opcional          │
├─────────────────┼────────────────────────────────────────────┤
│ Tipos atuador   │ mosfet_pwm, mosfet_aod4184, ssr,           │
│                 │ fan_pwm, fan_digital, shared_mosfet,       │
│                 │ stepper, servo, gpio, custom               │
│                 │ [triac REJEITADO]                          │
├─────────────────┼────────────────────────────────────────────┤
│ Display (9+2)   │ st7789, ili9341, st7735, gc9a01, ili9488,  │
│                 │ ssd1306, sh1106, hd44780, nextion,        │
│                 │ auto (auto-probe), none                    │
├─────────────────┼────────────────────────────────────────────┤
│ Touch (5)       │ resistivo:  xpt2046, stmpe610              │
│                 │ capacitivo: ft6236, gt911, cst816s         │
│                 │ auto (I2C primeiro, depois SPI), none      │
├─────────────────┼────────────────────────────────────────────┤
│ Controle (3)    │ pid, bang_bang, pwm_feedforward            │
│                 │ [custom REJEITADO até plugin]              │
├─────────────────┼────────────────────────────────────────────┤
│ Buses (5)       │ i2c, spi, onewire, uart, adc               │
│                 │ [parallel_8bit REJEITADO]                  │
├─────────────────┼────────────────────────────────────────────┤
│ Boards (3)      │ ESP32WROOM-32U (esp32dev)                  │
│                 │ LILYGO T-Display (ttgo-t-display)          │
│                 │ ESP32-2432S028 CYD (esp32_2432s028)        │
└─────────────────┴────────────────────────────────────────────┘
```

## Observações críticas

- **Não existe cooler de resfriamento separado** — o único fan é o exaustão, que também roda no COOLDOWN (30s pós-ciclo).
- **Conflito CYD**: backlight ILI9341=GPIO21 colide com I2C SDA=GPIO21 — precisa remapear um dos dois.
- **Touch CYD**: XPT2046 não tem pinos default no código — precisa JSON de configuração explícito.
- Todos os 3 boards compartilham os mesmos pinos de I2C/heater/fan — a diferença é só o display onboard e touch.
