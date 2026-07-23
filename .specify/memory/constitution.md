<!--
Sync Impact Report:
Version change: 0.3.0 → 0.4.0 (MINOR - explicit commit approval governance rule)
Modified principles: Git workflow requirements updated
Added sections: Explicit Commit Approval Rule
Removed sections: None
Templates requiring updates: 
  - .specify/templates/plan-template.md (✅ aligned)
  - .specify/templates/spec-template.md (✅ aligned)
  - .specify/templates/tasks-template.md (✅ aligned)
Follow-up TODOs: None
-->

# Philarmony Constitution

## Core Principles

### Cavemen Protocol (Token Efficiency)
All AI agents MUST communicate in minimal, telegraphic language. No filler phrases ("Of course!", "Sure!", "Here's what I did", "Great question"). No preamble before code. No post-code summaries unless explicitly requested. Prose is limited to what is strictly necessary for correctness. Code output is always complete and untruncated. Responses MUST be 1–3 sentences when no code is involved. If a question can be answered with a word or a number, use only that. Agents MUST apply this principle to ALL outputs: analysis reports, completion reports, inline comments in command files, and conversational turns.

### Revisão e Aprovação Explícita de Commit
Nenhum commit Git MUST ser criado automaticamente por agentes, hooks ou scripts sem que o usuário tenha tido chance explícita de revisar o código e autorizar a gravação.

Regras não negociáveis:
- Após concluir uma etapa (specify/plan/tasks/implement/constitution/etc.), o agente MUST apresentar: (1) resumo do que mudou, (2) lista dos arquivos principais, (3) mensagem de commit sugerida (Conventional Commits), (4) pergunta clara pedindo autorização.
- O agente MUST aguardar resposta afirmativa explícita do usuário (ex.: "sim", "commit", "autorizo", "pode commitar") antes de executar `git add`/`git commit` ou qualquer hook que grave no Git.
- Ausência de TTY, timeout, falha de leitura, ambiente não interativo ou prompt sem resposta MUST resultar em **não commitar** (fail-closed). Nunca interpretar silêncio, erro de `/dev/tty` ou default implícito como "sim".
- Hooks de commit Speckit (`git-commit-hook.sh` e equivalentes) MUST ser opcionais na experiência do agente: o agente ofereceece a sugestão e só executa o hook/comando se o usuário autorizar.
- "Revisar o código" inclui permitir que o usuário inspecione o diff no IDE/chat antes da autorização; o agente NÃO MUST pular essa etapa.

Rationale: commits irreversíveis no histórico local/remoto sem revisão violam a confiança do fluxo Speckit e da governança do projeto.

### Orientação a Objetos e Segurança de Hardware
Todos os componentes devem ser organizados usando princípios de orientação a objetos. Interfaces claras e tipagem forte garantem a segurança de dados e a modularidade. Funções que interagem diretamente com hardware devem estar encapsuladas em objetos com validação e safe abortos para parâmetros fora de controle (ex: temperatura, tensão), garantindo proteção contra danos ao hardware e abortos limpos (sem hangs).

### Desempenho Máximo e Eficiência
Code MUST be optimized for ESP32's constrained resources. Use deterministic algorithms, avoid dynamic allocations, and implement profiling. Each function MUST have a clear performance contract (timing, memory). Blocking operations are forbidden. Use hardware-specific APIs where necessary for maximum throughput.

### Failsafe e Proteção de Hardware
All hardware-control routines MUST include parameter validation that aborts on out-of-bounds values. Implement watchdog timers and safe fallback states. MUST detect error conditions within defined timeouts and transition to safe operating states automatically, preventing hardware damage.

### Workspace de Dependências e Configurações
A shared workspace configuration file (.vscode/workspace.json) MUST be maintained for consistent toolchains across machines. All dependencies listed in requirements.txt/package.json. Each new development tool MUST be added to this workspace file with proven documentation and licensing, ensuring reproducibility.

### Teste Automatizado e Qualidade
Automation testing pipeline is NON-NEGOTIABLE. Tests represent complete user flows only. CI system MUST fail build if any test fails. Test results cached in memory catalog for AI reference. Only flow tests (not unit tests) are mandatory, each representing complete user journeys.

### Documentação Sincronizada (README Vivo)
O arquivo README.md na raiz do projeto DEVE ser mantido automaticamente sincronizado com o estado atual do projeto. Qualquer comando speckit que crie, modifique ou remova especificações (`/speckit.specify`, `/speckit.plan`, `/speckit.tasks`) DEVE atualizar o README para refletir: progresso das fases, especificações completadas/em andamento, roadmap atualizado, stack tecnológico, estrutura do repositório e próximos passos. O README é a "single source of truth" para status do projeto visível a contribuidores e usuários.

### Memória Compartilhada de Pesquisa e Conhecimento
Todos os dados pesquisados, descobertas técnicas, decisões de arquitetura, referências de APIs, esquemas de hardware, e lições aprendidas DEVEM ser persistidos no catálogo de memória compartilhada da aplicação (shared memory catalog). Este catálogo serve como base de conhecimento única e consultável por agentes IA, desenvolvedores e ferramentas de automação. A escrita no catálogo é obrigatória em:
- Conclusão de tarefas de pesquisa (`/speckit.research` ou equivalente)
- Decisões de arquitetura documentadas em specs/plans
- Descobertas durante debugging ou integração
- Referências de APIs externas, datasheets, protocolos
- Padrões de código validados e anti-padrões identificados
O catálogo deve ser versionado, consultável via query semântica, e sincronizado entre sessões de desenvolvimento.

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
- Conventional Commits: All commit messages MUST follow Conventional Commits format with feature scope: `tipo(<feature>): descrição` (ex: `docs(filament-dryer-esp32): Add implementation plan`).
- Confirmação Explícita de Commit: É PROIBIDO realizar commits automáticos sem a confirmação prévia e explícita do usuário. Após cada ação/etapa, o sistema ou agente DEVE apresentar o resumo das alterações e a mensagem sugerida de commit e aguardar autorização do usuário antes de executar a gravação no Git.
- Branch strategy: Create new branch (`feature/<nome-do-projeto>`) for each task block before implementation
- Incremental commits: Make commits between tasks after validation of tests and user explicit authorization
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
- **Memory Catalog Sync Rule**: Toda pesquisa, decisão arquitetural, descoberta técnica ou referência validada DEVE ser registrada no catálogo de memória compartilhada. Falha no registro bloqueia merge de PRs que introduzam novo conhecimento.
- **Commit Approval Rule**: Nenhum commit será efetuado sem aprovação explícita do usuário via prompt interativo no final de cada ação.

**Version**: 0.4.0 | **Ratified**: 2026-07-22 | **Last Amended**: 2026-07-23
