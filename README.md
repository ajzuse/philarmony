# Philarmony - Secadora de Filamentos DIY ESP32

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Language: C/C++](https://img.shields.io/badge/Language-C%2FC%2B%2B-orange.svg)]()
[![Spec Kit](https://img.shields.io/badge/Spec%20Kit-v0.13.2-green.svg)]()
[![Constitution](https://img.shields.io/badge/Constitution-v0.6.0-purple.svg)](.specify/memory/constitution.md)

## 📋 Sobre o Projeto

**Philarmony** é uma plataforma open-source para secadora de filamentos DIY controlada por ESP32. O projeto implementa controle de temperatura, umidade, tempo de secagem, aquecedor e ventilação com foco em **segurança (failsafe)**, **performance máxima** e **arquitetura orientada a objetos**.

### Características Principais

- 🛡️ **Failsafe Total**: Watchdog, limites de temperatura, corte de segurança, fallbacks automáticos
- ⚡ **Performance Máxima**: Código otimizado para ESP32, sem alocações dinâmicas, algoritmos determinísticos
- 🔧 **OOP Hardware-Safe**: Interfaces claras, tipagem forte, encapsulamento de hardware com validação
- 🌐 **Conectividade Completa**: WiFi + Hotspot fallback + WebSocket API em tempo real
- 📱 **Multi-Plataforma**: App Desktop (Win/Mac/Linux) + Android + iOS + Interface Touch no dispositivo
- 🔧 **Instalador Desktop**: Configuração guiada + flash pré-configurado via USB
- 📦 **Perfis de Filamento**: Templates built-in (PLA, PETG, ABS, TPU, Nylon) + perfis customizados
- 🌍 **Internacionalização**: PT-BR (primário) + EN-US em toda documentação e UI

---

## 🏗️ Arquitetura do Sistema

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        PHILARMONY ECOSYSTEM                              │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────────┐    ┌──────────────────┐    ┌──────────────────┐  │
│  │  ESP32 FIRMWARE  │◄──►│  CONTROL APP     │    │  DESKTOP         │  │
│  │  (001 + 004)     │    │  (003)           │    │  INSTALLER (002) │  │
│  │                  │    │                  │    │                  │  │
│  │ • Temp/Humidity  │    │ • Desktop App    │    │ • USB Flash      │  │
│  │ • Heater/Fan PWM │    │ • Mobile Apps    │    │ • Config Wizard  │  │
│  │ • WebSocket API  │    │ • Multi-device   │    │ • Profile Sync   │  │
│  │ • Hotspot Config │    │ • History/Export │    │ • NVS Injection  │  │
│  │ • Touch UI (004) │    │ • Filament Prof. │    │                  │  │
│  │ • Filament Prof. │    │                  │    │                  │  │
│  └────────┬─────────┘    └────────┬─────────┘    └────────┬─────────┘  │
│           │                       │                       │            │
│           │      WebSocket (1Hz)  │                       │            │
│           │◄──────────────────────►│                       │            │
│           │                       │                       │            │
│           ▼                       ▼                       ▼            │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │                    SHARED SPECIFICATIONS                          │  │
│  │  • FilamentProfile Schema  • WebSocket Protocol  • NVS Schema    │  │
│  │  • Status Payload            • Config Topics       • Safety Limits│  │
│  └──────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 📦 Especificações (Specs) - Status Atual

| # | Spec | Nome | Status | Última Atualização |
|---|------|------|--------|-------------------|
| 1 | `001-filament-dryer-esp32` | Filament Dryer ESP32 Base Structure   | ⚙️ Tasks Geradas | 2026-07-23   |
| 2 | `002-esp32-desktop-installer` | ESP32 Desktop Installer   | 📝 Especificado | 2026-07-23   |
| 3 | `003-filament-dryer-control-app` | Filament Dryer Control App   | 📝 Especificado | 2026-07-23   |
| 4 | `004-esp32-touchscreen-ui` | ESP32 Touchscreen Interface   | 📝 Especificado | 2026-07-23   |

---

## 🎯 Subprojetos Detalhados

### 001 - ESP32 Firmware Base (`specs/001-filament-dryer-esp32/`)

**Objetivo**: Firmware principal rodando no ESP32 controlando todos os aspectos da secadora.

#### Funcionalidades Implementadas na Spec

| Área | Detalhes |
|------|----------|
| **WiFi & Conectividade** | Conexão WiFi com fallback automático para Hotspot "philarmony"/"philarmony" (IP fixo 192.168.4.1) com servidor HTTP para configuração |
| **WebSocket API** | Servidor WebSocket na porta **80** path  com tópicos: `config/hardware`, `config/display`, `config/control`, `control/start`, `control/stop`, `control/pid_calibrate`, `status/subscribe`, `status/update`, `config/profiles/*`, `logs/stream` |
| **Controle Térmico** | PWM heater (0-100%), ventoinha exaustão PWM/digital, PID opcional, limite segurança 80°C hardcoded |
| **Sensores** | DHT22, DS18B20, BME280 configuráveis via GPIO |
| **Display** | SSD1306, SH1106 (I2C), ST7789, ILI9341 (SPI) - resolução configurável, campos selecionáveis |
| **Perfis de Filamento** | 5 built-in (PLA, PETG, ABS, TPU, Nylon) + até 20 customizados em NVS |
| **Status Streaming** | 1Hz via WebSocket: temp, humidity, heater%, fan%, CPU%, RAM, uptime, elapsed/remaining |
| **Segurança** | Watchdog HW, corte térmico 80°C, fan 30s pós-aquecimento, NVS atômico |

#### Entidades Principais
- `DryingCycle` - Ciclo de secagem com stop_reason
- `SensorConfig` - Tipo e pinagem de sensores
- `PinConfig` - Mapeamento GPIOs (heater, fan, sensors, I2C, SPI)
- `DisplayConfig` - Driver, resolução, campos visíveis
- `FilamentProfile` - id, name_pt/en, target_temp, duration, target_humidity, is_builtin

#### Fora do Escopo (Fase Atual)
- Touch screen (spec 004)
- Multi-zona
- OTA updates
- Autenticação/usuários
- MQTT/Cloud
- Scheduler/agendamento

---

### 002 - Desktop Installer (`specs/002-esp32-desktop-installer/`)

**Objetivo**: Aplicação desktop (Windows/macOS/Linux) que elimina a necessidade de configuração manual via WebSocket.

#### Funcionalidades

| Feature | Descrição |
|---------|-----------|
| **Device Detection** | Detecção automática ESP32 via USB (CP210x, CH340, FTDI) + identificação modelo/flash |
| **Wizard Guiado** | 6 passos: Modelo → Sensores → Pinagem → Display → Perfis → WiFi → Review → Flash |
| **Validação Visual** | Pinout diagram interativo, detecção de conflitos, pins reservados destacados |
| **Perfis de Filamento** | Visualização/edição dos 5 built-in + criação customizados, sincronizados no flash |
| **Config Profiles** | Export/import JSON (senha WiFi como placeholder), versionamento |
| **Flash Process** | Erase → Write Firmware → Write Config NVS → Verify → Reset, com progress bar |
| **Multi-linguagem** | PT-BR / EN-US com strings externalizadas |

#### Entidades
- `DeviceProfile` - Modelo ESP32, flash, sensores, pins, display, WiFi, firmware_version
- `FilamentProfile` - Mesmo schema do firmware (id, names, temp, duration, humidity, is_builtin)
- `FirmwarePackage` - version, binary_path, partition_table, bootloader, checksum SHA256

---

### 003 - Control App Multi-Plataforma (`specs/003-filament-dryer-control-app/`)

**Objetivo**: App unificado Desktop + Mobile para monitoramento, configuração e controle remoto.

#### Plataformas Suportadas
- **Desktop**: Windows 10/11 (x64), macOS 12+ (Universal), Linux Ubuntu 20.04+ (AppImage/Flatpak)
- **Mobile**: Android 8+ (API 26), iOS 15+

#### Funcionalidades Core

| Módulo | Features |
|--------|----------|
| **Discovery** | mDNS (`_philarmony._tcp`) + SSDP + IP manual, auto-connect último dispositivo |
| **Dashboard** | Status real-time 1Hz, charts scrolling 60min (temp, humidity, heater%), badges de estado |
| **Controle Ciclo** | Start com profile_id OU parâmetros custom, Stop com confirmação, ajustes mid-cycle |
| **Configuração Remota** | Sensors, Pin Mapping (visual), Display, WiFi (trigger hotspot), Advanced (read-only safety) |
| **Histórico** | SQLite local, 50+ ciclos, filtros (data, material, device), export CSV/PDF, sparklines |
| **Multi-Device** | Registry com nickname, quick-switch header, conexões simultâneas (default 3) |
| **Notificações** | In-app toasts + Push nativo (FCM/APNs opcional) + Desktop system notifications |
| **Perfis Material** | 5 built-in + custom CRUD, one-tap "Start with Profile" |
| **Offline-First** | Local DB, command queue, sync on reconnect, conflict resolution (device wins config) |

#### Entidades Compartilhadas
- `DeviceProfile` - nickname, host, last_seen, firmware_version, auto_connect
- `DryingCycle` - UUID, device_id, profile_id, material_name, targets, actuals, duration, stop_reason, data_points[]
- `MaterialProfile` - UUID, names, targets, is_builtin, created_by_user

---

### 004 - Touchscreen UI On-Device (`specs/004-esp32-touchscreen-ui/`)

**Objetivo**: Interface touch nativa no ESP32 para operação standalone completa.

#### Suportado
- **Resistivo**: XPT2046, STMPE610 (SPI/I2C)
- **Capacitivo**: FT6236, GT911, CST816S (I2C)
- Auto-detecção via probe I2C/SPI

#### Telas (State Machine)

```
[Boot] → [Splash] → [Home/Idle]
                    ↓
         ┌──────────┼──────────┐
         ↓          ↓          ↓
      [Start]    [Settings]  [History]
        ↓          ↓           ↓
    [Presets]  [Categories] [List]
      ↓          ↓           ↓
    [Custom]  [Options]    [Detail]
      ↓          ↓
   [Confirm]  [Save]     ←←←←←←←←
      ↓
 [Monitoring] ←←←←←←←←←←←←←←←←←←
      ↓
    [Stop] → [Confirm] → [Home]
```

#### Funcionalidades Touch

| Feature | Detalhes |
|---------|----------|
| **Start Flow** | 3 taps: Home → Material → Confirm (preset) ou Home → Custom → Params → Confirm |
| **Monitoring** | Valores grandes, progress ring, Stop button, tap values para ajustar mid-cycle |
| **Settings** | WiFi (reconfig→hotspot), Display (brightness, timeout, orientation), Units (°C/°F), Language, Touch (sensibilidade, calibração 4-pontos), Advanced (device name, firmware, reset) |
| **History** | 50 ciclos circular buffer NVS/SPIFFS, list + detail com sparklines, export via WS/CSV |
| **Sync WS** | Estado compartilhado, `ui_source` no status, config ack broadcast, last-write-wins |
| **Acessibilidade** | Targets 48x48px, high contrast mode, haptic/beep feedback, PIN lock opcional |

#### Entidades
- `TouchConfig` - controller_type, pins, calibration, sensitivity, swap/invert axes
- `UIScreen` - state machine enum + params
- `MaterialPreset` - 5 built-in + custom (mesmo schema FilamentProfile)
- `CycleRecord` - 50 entries circular buffer
- `UISettings` - brightness, timeout, orientation, units, language, touch, contrast, pin_lock

---

## 📊 Progresso Geral do Projeto

```
┌────────────────────────────┬────────────┬────────────────────────┬──────┐
│ FASE                       │ STATUS     │ ENTREGÁVEIS            │  %   │
├────────────────────────────┼────────────┼────────────────────────┼──────┤
│ 1. Specification (Speckit) │ ✅ CONCLUÍDO │ 4 Specs completas    │ 100% │
│ 2. Planning (Speckit)      │ 🔄 EM ANDAMENTO │ Plan.md + Tasks por spec │ 1/4  │
│ 3. Firmware Core           │ ⏳ AGUARDANDO │ ESP32 Base + WS + NVS   │ 0%   │
│ 4. Touch UI                │ ⏳ AGUARDANDO │ LVGL + Touch Driver     │ 0%   │
│ 5. Desktop Installer       │ ⏳ AGUARDANDO │ Tauri/Flutter + esptool │ 0%   │
│ 6. Control App (Multi)     │ ⏳ AGUARDANDO │ Flutter + SQLite + WS   │ 0%   │
│ 7. Integration & Testing   │ ⏳ FUTURO    │ E2E, CI/CD, Release     │ 0%   │
└────────────────────────────┴────────────┴────────────────────────┴──────┘
```

---

## 🔧 Stack Tecnológico Definido

| Camada | Tecnologia | Justificativa |
|--------|------------|---------------|
| **Firmware** | ESP-IDF / Arduino Core | Oficial Espressif, suporte completo ESP32 |
| **GUI Firmware** | LVGL v8+ | Leve, touch-ready, double buffer, temas |
| **WebSocket** | async_web_server / ESPAsyncWebServer | Non-blocking, multi-client, 1Hz nativo |
| **NVS/Storage** | NVS (config) + SPIFFS (fonts/assets) | Wear-leveling, atômico, padrão ESP32 |
| **Desktop Installer** | Tauri (Rust + Web) ou Flutter Desktop | Binário nativo pequeno, esptool.py integrado |
| **Control App** | Flutter 3.x (Dart) | Single codebase Desktop+Mobile, SQLite, WS |
| **Build/CI** | GitHub Actions + PlatformIO + GNU Make | `make build` / `make test` / `make flash` (Phase 20 tasks) |
| **Docs** | Markdown PT-BR/EN-US em `/docs` | GitHub Pages ready, versionado com código |

---

## 🛠️ Makefile

```bash
make help
make build                 # ENV=esp32devkitc (lilygo_tdisplay_v1 | esp32_2432s028)
make test                  # pio test -e native
make flash PORT=/dev/cu.usbserial-*   # precisa ESP32; aliases: upload, install
```

Contrato: `specs/001-filament-dryer-esp32/contracts/makefile-targets.md`.

---

## 📁 Estrutura do Repositório (Planejada)

```
philarmony/
├── .specify/                    # Spec Kit configuração
│   ├── memory/constitution.md   # Constituição do projeto
│   ├── templates/               # Templates spec/plan/tasks
│   ├── scripts/                 # Scripts de automação (sync-readme, etc.)
│   └── extensions.yml           # Hooks para sincronização automática
├── .vscode/
│   └── workspace.json
├── docs/
│   ├── PT-BR/
│   ├── EN-US/
├── src/
│   ├── hardware/
│   ├── software/
│   └── lib/
├── tests/
│   ├── flow/
│   └── integration/
├── examples/
├── specs/                       # Especificações (4 atuais)
│   ├── 001-filament-dryer-esp32/
│   ├── 002-esp32-desktop-installer/
│   ├── 003-filament-dryer-control-app/
│   └── 004-esp32-touchscreen-ui/
├── README.md                    # Este arquivo (auto-sincronizado)
└── LICENSE
```

---

## 🛠️ Habilitando o Workspace

```bash
# Abra o projeto no VS Code
# A configuração compartilhada do workspace será aplicada automaticamente
# Todas as extensões e definições no .vscode/ estarão disponíveis
```

---

## 🧪 Testes

Todo o desenvolvimento deve seguir as práticas de Test-First (TDD) sempre que possível. Se o TDD não for possível, apenas os testes de fluxo completo são permitidos.

---

## 📝 Changelog

Todas as alterações de conteúdo devem ser documentadas no CHANGELOG.md, seguindo as convenções de commit convencionais.

---

## 🤝 Contribuição

Para contribuir, por favor siga estas instruções:

1. Faça um fork deste repositório
2. Crie um branch para seu recurso (`git checkout -b feature/xxx`)
3. Faça commit de suas alterações (`git commit -a` - **Conventional Commits obrigatório**)
4. Faça push para o branch (`git push origin feature/xxx`)
5. Abra um Pull Request

### Requisitos de PR
- ✅ Dois revisores aprovando
- ✅ Testes de fluxo passando
- ✅ Conformidade constitucional verificada
- ✅ README sincronizado (automático via hooks)

---

## 📖 Documentação

Consulte a documentação em `docs/` para guias detalhados sobre configuração, uso e desenvolvimento do projeto.

- **PT-BR**: `docs/PT-BR/`
- **EN-US**: `docs/EN-US/`

---

## 🔒 Segurança

Todos os commits devem seguir as convenções convencionais de commit para garantir um histórico rastreável e de qualidade.

**Failsafe**: Qualquer violação de parâmetros de segurança (temperatura > 80°C, sensor offline, watchdog) resulta em corte imediato do aquecedor e ventilação forçada por 30s.

---

## 📄 Licença

Este projeto é licenciado sob a **GNU General Public License v3.0**

Copyright © 2024-2026 ESP32 DIY Hardware Team

Todos têm permissão para copiar e distribuir cópias verbatim deste documento de licença, mas alterá-lo não é permitido.

Este programa é software livre: você pode redistribuí-lo e/ou modificá-lo sob os termos da GNU General Public License como publicada pela Free Software Foundation, seja a versão 3 da Licença, ou (a seu critério) qualquer versão posterior.

Este programa é distribuído na esperança de que será útil, mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM PROPÓSITO ESPECÍFICO. Veja a GNU General Public License para mais detalhes.

Você deve ter recebido uma cópia da GNU General Public License junto com este programa. Se não, veja <https://www.gnu.org/licenses/>.

---

*README auto-gerado e sincronizado pela Constituição Philarmony v0.6.0*
*Última atualização: 2026-07-28 | Trigger: tasks*
