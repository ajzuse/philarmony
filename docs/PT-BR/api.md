# Philarmony — Documentação da API (PT-BR)

## Visão geral

Firmware ESP32 para secadora de filamento DIY. Controle e telemetria via WebSocket em `ws://<ip>/ws`. Configuração WiFi inicial via hotspot `philarmony` / `philarmony` em `http://192.168.4.1/`.

## Tópicos WebSocket principais

| Tópico | Direção | Descrição |
|--------|---------|-----------|
| `control/start` | cliente→ESP | Inicia ciclo (perfil ou parâmetros) |
| `control/stop` | cliente→ESP | Interrompe aquecimento/ventilação |
| `status/subscribe` | cliente→ESP | Assina telemetria 1 Hz |
| `status/update` | ESP→cliente | Status (temp, umidade, PWM, métricas) |
| `status/fault` | ESP→cliente | Falha com `fault_code` |
| `config/hardware` | cliente→ESP | Sensores, atuadores, display, controle |
| `config/display` | cliente→ESP | Layout e taxa de atualização |
| `config/profiles/*` | cliente→ESP | CRUD de perfis de filamento |
| `control/pid_calibrate` | cliente→ESP | Auto-tune PID (campo `cycles` ou `max_cycles`) |

## Segurança

Limite térmico configurável (padrão 80 °C), watchdog, corte de emergência dos atuadores e logs em `/log/system` e `/log/drying`.

## Arquitetura

Ver [arquitetura.md](arquitetura.md).
