# Philarmony - Secadora de Filamentos DIY ESP32

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Language: C/C++](https://img.shields.io/badge/Language-C%2FC%2B%2B-orange.svg)]()
[![Spec Kit](https://img.shields.io/badge/Spec%20Kit-v0.13.2-green.svg)]()
[![Constitution](https://img.shields.io/badge/Constitution-v0.3.0-purple.svg)](.specify/memory/constitution.md)

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

**CONSTITUIÇÃO v${constitution_version}** | Última emenda: ${constitution_amended} | Atualizado: ${current_date}

${specs_table}

---

## 🗺️ Roadmap do Projeto

${roadmap}

---

## 🎯 Próximos Passos Imediatos

${next_steps}

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
| **Build/CI** | GitHub Actions + PlatformIO | Matrix build ESP32 variants, artifact upload |
| **Docs** | Markdown PT-BR/EN-US em \`/docs\` | GitHub Pages ready, versionado com código |

---

## 📁 Estrutura do Repositório (Planejada)

\`\`\`
philarmony/
├── .specify/                    # Spec Kit configuração
│   ├── memory/constitution.md   # Constituição do projeto
│   ├── templates/               # Templates spec/plan/tasks
│   ├── scripts/                 # Scripts de automação (sync-readme, etc.)
│   └── extensions.yml           # Hooks de sincronização
├── .vscode/
│   └── workspace.json           # Workspace compartilhado VS Code
├── docs/
│   ├── PT-BR/
│   ├── EN-US/
├── specs/                       # Especificações (geradas pelo Speckit)
│   ├── 001-filament-dryer-esp32/
│   ├── 002-esp32-desktop-installer/
│   ├── 003-filament-dryer-control-app/
│   └── 004-esp32-touchscreen-ui/
├── src/
│   ├── firmware/                # Código ESP32 (PlatformIO/ESP-IDF)
│   ├── installer/               # Desktop Installer (Tauri/Flutter)
│   └── app/                     # Control App (Flutter)
├── tests/
│   ├── flow/
│   └── integration/
├── examples/
├── README.md                    # ESTE ARQUIVO (auto-sincronizado)
├── CHANGELOG.md
└── LICENSE
\`\`\`

---

## 🔄 Sincronização Automática (README Vivo)

> **Constituição v0.3.0 - Princípios: Documentação Sincronizada & Memória Compartilhada**
> 
> Este README é **automaticamente atualizado** a cada comando Speckit que altere o estado do projeto:
> - \`/speckit.specify\` → Nova/atualizada spec → Roadmap, tabela de specs, próximos passos
> - \`/speckit.plan\` → Plano gerado → Fase de planning, tasks identificadas
> - \`/speckit.tasks\` → Tasks detalhadas → Breakdown de implementação, estimativas
> 
> **Implementação**: Hooks em \`.specify/extensions.yml\` → \`sync-readme.sh\`

### Última Sincronização
- **Trigger**: ${TRIGGER}
- **Data**: ${current_date}
- **Specs**: ${spec_count} | **Planos**: ${plan_count} | **Tasks**: $(find "$SPECS_DIR" -name "tasks.md" 2>/dev/null | wc -l | tr -d ' ')

---

## 📚 Subprojetos Detalhados

### 001 - ESP32 Firmware Base (\`specs/001-filament-dryer-esp32/\`)

**Objetivo**: Firmware principal rodando no ESP32 controlando todos os aspectos da secadora.

| Área | Detalhes |
|------|----------|
| **WiFi & Conectividade** | Conexão WiFi com fallback automático para Hotspot "philarmony"/"philarmony" (IP fixo 192.168.4.1) com servidor HTTP para configuração |
| **WebSocket API** | Servidor WebSocket na porta 8080 com tópicos: \`config/sensors\`, \`config/pins\`, \`config/display\`, \`control/start\`, \`control/stop\`, \`status/subscribe\`, \`config/profiles/*\` |
| **Controle Térmico** | PWM heater (0-100%), ventoinha exaustão PWM/digital, PID opcional, limite segurança 80°C hardcoded |
| **Sensores** | DHT22, DS18B20, BME280 configuráveis via GPIO |
| **Display** | SSD1306, SH1106 (I2C), ST7789, ILI9341 (SPI) - resolução configurável, campos selecionáveis |
| **Perfis de Filamento** | 5 built-in (PLA, PETG, ABS, TPU, Nylon) + até 20 customizados em NVS |
| **Status Streaming** | 1Hz via WebSocket: temp, humidity, heater%, fan%, CPU%, RAM, uptime, elapsed/remaining |
| **Segurança** | Watchdog HW, corte térmico 80°C, fan 30s pós-aquecimento, NVS atômico |

---

### 002 - Desktop Installer (\`specs/002-esp32-desktop-installer/\`)

**Objetivo**: Aplicação desktop (Windows/macOS/Linux) que elimina a necessidade de configuração manual via WebSocket.

| Feature | Descrição |
|---------|-----------|
| **Device Detection** | Detecção automática ESP32 via USB (CP210x, CH340, FTDI) + identificação modelo/flash |
| **Wizard Guiado** | 6 passos: Modelo → Sensores → Pinagem → Display → Perfis → WiFi → Review → Flash |
| **Validação Visual** | Pinout diagram interativo, detecção de conflitos, pins reservados destacados |
| **Perfis de Filamento** | Visualização/edição dos 5 built-in + criação customizados, sincronizados no flash |
| **Config Profiles** | Export/import JSON (senha WiFi como placeholder), versionamento |
| **Flash Process** | Erase → Write Firmware → Write Config NVS → Verify → Reset, com progress bar |
| **Multi-linguagem** | PT-BR / EN-US com strings externalizadas |

---

### 003 - Control App Multi-Plataforma (\`specs/003-filament-dryer-control-app/\`)

**Objetivo**: App unificado Desktop + Mobile para monitoramento, configuração e controle remoto.

| Plataforma | Suporte |
|------------|---------|
| **Desktop** | Windows 10/11 (x64), macOS 12+ (Universal), Linux Ubuntu 20.04+ (AppImage/Flatpak) |
| **Mobile** | Android 8+ (API 26), iOS 15+ |

| Módulo | Features |
|--------|----------|
| **Discovery** | mDNS (\`_philarmony._tcp\`) + SSDP + IP manual, auto-connect último dispositivo |
| **Dashboard** | Status real-time 1Hz, charts scrolling 60min (temp, humidity, heater%), badges de estado |
| **Controle Ciclo** | Start com profile_id OU parâmetros custom, Stop com confirmação, ajustes mid-cycle |
| **Configuração Remota** | Sensors, Pin Mapping (visual), Display, WiFi (trigger hotspot), Advanced (read-only safety) |
| **Histórico** | SQLite local, 50+ ciclos, filtros (data, material, device), export CSV/PDF, sparklines |
| **Multi-Device** | Registry com nickname, quick-switch header, conexões simultâneas (default 3) |
| **Notificações** | In-app toasts + Push nativo (FCM/APNs opcional) + Desktop system notifications |
| **Perfis Material** | 5 built-in + custom CRUD, one-tap "Start with Profile" |
| **Offline-First** | Local DB, command queue, sync on reconnect, conflict resolution (device wins config) |

---

### 004 - Touchscreen UI On-Device (\`specs/004-esp32-touchscreen-ui/\`)

**Objetivo**: Interface touch nativa no ESP32 para operação standalone completa.

| Tipo | Controllers Suportados |
|------|------------------------|
| **Resistivo** | XPT2046, STMPE610 (SPI/I2C) |
| **Capacitivo** | FT6236, GT911, CST816S (I2C) |
| **Auto-detecção** | Via probe I2C/SPI |

#### Telas (State Machine)

\`\`\`
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
\`\`\`

| Feature | Detalhes |
|---------|----------|
| **Start Flow** | 3 taps: Home → Material → Confirm (preset) ou Home → Custom → Params → Confirm |
| **Monitoring** | Valores grandes, progress ring, Stop button, tap values para ajustar mid-cycle |
| **Settings** | WiFi (reconfig→hotspot), Display (brightness, timeout, orientation), Units (°C/°F), Language, Touch (sensibilidade, calibração 4-pontos), Advanced (device name, firmware, reset) |
| **History** | 50 ciclos circular buffer NVS/SPIFFS, list + detail com sparklines, export via WS/CSV |
| **Sync WS** | Estado compartilhado, \`ui_source\` no status, config ack broadcast, last-write-wins |
| **Acessibilidade** | Targets 48x48px, high contrast mode, haptic/beep feedback, PIN lock opcional |

---

## 📊 Progresso Geral do Projeto

\`\`\`
┌────────────────────────────────────────────────────────────────────┐
│                      PROJECT ROADMAP                                │
├──────────────────┬──────────────────┬────────────────────────┬──────┤
│     FASE         │     STATUS       │   ENTREGÁVEIS            │  %   │
├──────────────────┼──────────────────┼────────────────────────┼──────┤
│ 1. Specification │ ✅ CONCLUÍDO     │ 4 Specs completas      │ 100% │
│    (Speckit)     │                  │ + Filament Prof.       │      │
├──────────────────┼──────────────────┼────────────────────────┼──────┤
│ 2. Planning      │ ⏳ PRÓXIMO       │ Plan.md + Tasks        │  0%  │
│    (Speckit)     │                  │ por spec               │      │
├──────────────────┼──────────────────┼────────────────────────┼──────┤
│ 3. Firmware Core │ ⏳ AGUARDANDO    │ ESP32 Base +           │  0%  │
│    Implementation│    PLANNING      │ WebSocket + NVS        │      │
├──────────────────┼──────────────────┼────────────────────────┼──────┤
│ 4. Touch UI      │ ⏳ AGUARDANDO    │ LVGL + Touch           │  0%  │
│    Implementation│    PLANNING      │ Driver + Screens       │      │
├──────────────────┼──────────────────┼────────────────────────┼──────┤
│ 5. Desktop       │ ⏳ AGUARDANDO    │ Tauri/Flutter +        │  0%  │
│    Installer     │    PLANNING      │ esptool + Wizard       │      │
├──────────────────┼──────────────────┼────────────────────────┼──────┤
│ 6. Control App   │ ⏳ AGUARDANDO    │ Flutter +              │  0%  │
│    (Multi-plat)  │    PLANNING      │ SQLite + WS            │      │
├──────────────────┼──────────────────┼────────────────────────┼──────┤
│ 7. Integration   │ ⏳ FUTURO        │ E2E Testing,           │  0%  │
│    & Testing     │                  │ CI/CD, Release         │      │
└──────────────────┴──────────────────┴────────────────────────┴──────┘
\`\`\`

---

## 📄 Licença

Este projeto é licenciado sob a **GNU General Public License v3.0**

Copyright © 2024-2026 ESP32 DIY Hardware Team

Todos têm permissão para copiar e distribuir cópias verbatim deste documento de licença, mas alterá-lo não é permitido.

Este programa é software livre: você pode redistribuí-lo e/ou modificá-lo sob os termos da GNU General Public License como publicada pela Free Software Foundation, seja a versão 3 da Licença, ou (a seu critério) qualquer versão posterior.

Este programa é distribuído na esperança de que será útil, mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM PROPÓSITO ESPECÍFICO. Veja a GNU General Public License para mais detalhes.

Você deve ter recebido uma cópia da GNU General Public License junto com este programa. Se não, veja <https://www.gnu.org/licenses/>.

---

## 🤝 Contribuição

Para contribuir, por favor siga estas instruções:

1. Faça um fork deste repositório
2. Crie um branch para seu recurso (\`git checkout -b feature/xxx\`)
3. Faça commit de suas alterações (\`git commit -a\` - **Conventional Commits obrigatório**)
4. Faça push para o branch (\`git push origin feature/xxx\`)
5. Abra um Pull Request

### Requisitos de PR
- ✅ Dois revisores aprovando
- ✅ Testes de fluxo passando
- ✅ Conformidade constitucional verificada
- ✅ README sincronizado (automático via hooks)

---

## 📖 Documentação

Consulte a documentação em \`docs/\` para guias detalhados sobre configuração, uso e desenvolvimento do projeto.

- **PT-BR**: \`docs/PT-BR/\`
- **EN-US**: \`docs/EN-US/\`

---

## 🔒 Segurança

Todos os commits devem seguir as convenções convencionais de commit para garantir um histórico rastreável e de qualidade.

**Failsafe**: Qualquer violação de parâmetros de segurança (temperatura > 80°C, sensor offline, watchdog) resulta em corte imediato do aquecedor e ventilação forçada por 30s.

---

*README auto-gerado e sincronizado pela Constituição Philarmony v${constitution_version}*
*Última atualização: ${current_date} | Trigger: ${TRIGGER}*
