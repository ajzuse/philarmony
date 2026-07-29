# Philarmony — Configuração (PT-BR)

## WiFi

Persistido em NVS via portal cativo ou `POST /api/wifi/config`. Credenciais inválidas/ausentes → AP no prazo fail-fast SC-01.

## Hardware (`config/hardware`)

Schema igual ao contrato EN-US / `contracts/config-schema.json`: arrays `sensors`/`actuators`, `display` aninhado, `control.safety_limits`.

### Regras

- Pinos de saída (PWM/CS/DC/RST/BL): GPIO **0–33** (34–39 rejeitados).
- `algorithm: "custom"` rejeitado até factory de plugin.
- `triac` / `parallel_8bit` rejeitados.
- Opcional: `current_sense_pin` no heater para `ACTUATOR_FAULT` real.

## Perfis

Built-ins: pla, petg, abs, tpu, nylon. Até 20 custom. Faixas: 30–80 °C, 1–1440 min, umidade 5–50%.

## Build / flash (Makefile)

Comandos preferidos no host (ver `specs/001-filament-dryer-esp32/contracts/makefile-targets.md`):

```bash
make build
make test
make flash PORT=/dev/ttyUSB0   # precisa de ESP32 conectada; aliases: upload, install
make help
```
