# Philarmony Constitution

## Core Principles

### Orientação a Objetos e Segurança de Hardware
Todos os componentes devem ser organizados usando princípios de orientação a objetos. Interfaces claras e tipagem forte garantem a segurança de dados e a modularidade. Funções que interagem diretamente com hardware devem estar encapsuladas em objetos com validação e safe abortos para parâmetros fora de controle (ex: temperatura, tensão), garantindo proteção contra danos ao hardware e abortos limpos (sem hangs).

### Desempenho Máximo e Eficiência
Code must be optimized for ESP32's constrained resources. Use deterministic algorithms, avoid dynamic allocations, and implement profiling. Each function must have a clear performance contract (timing, memory). Blocking operations are forbidden. Use hardware-specific APIs where necessary for maximum throughput.

### Failsafe e Proteção de Hardware
All hardware-control routines MUST include parameter validation that aborts on out-of-bounds values. Implement watchdog timers and safe fallback states. Must detect error conditions within defined timeouts and transition to safe operating states automatically, preventing hardware damage.

### Workspace de Dependências e Configurações
A shared workspace configuration file (.vscode/workspace.json) must be maintained for consistent toolchains across machines. All dependencies listed in requirements.txt/package.json. Each new development tool must be added to this workspace file with proven documentation and licensing, ensuring reproducibility.

### Teste Automatizado e Qualidade
Automation testing pipeline is NON-NEGOTIABLE. Tests represent complete user flows only. CI system MUST fail build if any test fails. Test results cached in memory catalog for AI reference. Only flow tests (not unit tests) are mandatory, each representing complete user journeys.

### Documentação Sincronizada (README Vivo)
O arquivo README.md na raiz do projeto DEVE ser mantido automaticamente sincronizado com o estado atual do projeto. Qualquer comando speckit que crie, modifique ou remova especificações (`/speckit.specify`, `/speckit.plan`, `/speckit.tasks`) DEVE atualizar o README para refletir: progresso das fases, especificações completadas/em andamento, roadmap atualizado, stack tecnológico, estrutura do repositório e próximos passos. O README é a "single source of truth" para status do projeto visível a contribuidores e usuários.

## Additional Constraints

**Technology stack requirements, compliance standards, deployment policies, etc.:**
- Core technology: ESP32, C/C++ (Arduino/PlatformIO)
- Memory constraints: Maximum 4MB RAM usage, must be validated on target hardware
- Performance targets: 100ms maximum response time for critical operations
- Testing: Automated flow testing only, no mocks of hardware unless specifically tested
- Compliance: Must meet all safety standards for DIY equipment
- Deployment: Pre-release builds must pass hardware simulation tests

## Development Workflow

**Code review requirements, testing gates, deployment approval process, etc.:**
- Code review process: Pair programming mandatory for all hardware interaction code
- Code review gates: All PRs must have ✅ from two reviewers
- Flow testing gates: Flow tests MUST pass before merge
- CI pipeline: Automated tests must pass for all PRs
- Deployment approval: Release manager approval required for production releases
- Version tagging: Semantic versioning with patch for security bugs, minor for features

**Git workflow requirements:**
- Conventional Commits: All commit messages MUST follow Conventional Commits format (e.g., "feat: add new sensor driver", "fix: handle sensor timeout")
- Branch strategy: Create new branch (feature/xxx) for each task block before implementation
- Incremental commits: Make commits between tasks after validation of tests
- Pull Requests: Open PR at end of each task block execution

## Language Support

**PT-BR Documentation:**
Documentação completa em PT-BR exigida para todos os componentes. Leituras de sensores, controles e interfaces devem estar disponíveis em português. Documentação inclui:
- API references em PT-BR
- Exemplos de código em PT-BR
- Documentação de arquitectura em PT-BR
- Documentação de configuração em PT-BR

**EN-US Documentation:**
Documentação completa em EN-US para contribuições open-source. Utilizada como documentação primária para a comunidade global. Inclui:
- API references em EN-US
- Exemplos de código em EN-US
- Documentação de arquitectura em EN-US
- Documentação de configuração em EN-US

## Governance

**All PRs/reviews must verify compliance; Complexity must be justified; Use README.md for runtime development guidance; README sync is mandatory on spec changes**
- Constituição supersedes all other practices; Amendments require documentation, approval, migration plan
- All PRs must verify constitutional compliance via automated checks
- GPLv3 license: All contributions must include full license header; source code must be freely redistributable
- Compliance review mandatory for any new dependencies or tools
- Complexity must be justified by performance or safety requirements
- Use README.md in docs/ for runtime development guidance
- **README Sync Rule**: Todo comando speckit que altere specs/plans/tasks DEVE invocar atualização do README.md como passo obrigatório. Falha na sincronização bloqueia merge. Implementação via hook `after_specify`/`after_plan`/`after_tasks` no `.specify/extensions.yml`.

**Version**: 0.2.0 | **Ratified**: 2026-07-22 | **Last Amended**: 2026-07-23