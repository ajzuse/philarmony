# Speckit Git Workflow Extension

Extension para o Speckit que automatiza o fluxo Git:
1. **Commit Convencional por Etapa**: Questiona após o término de cada etapa (`specify`, `plan`, `tasks`, `implement`) se deve ser feito um commit e gera a mensagem seguindo o padrão **Conventional Commits** (e validado via `commitlint`).
2. **Controle de Branch por Projeto Ativo**: Detecta mudanças no projeto ativo e questiona se deve ser criada ou alternada a branch git correspondente (`feature/<nome-do-projeto>`).

## Estrutura do Extension

```text
.specify/extensions/git-workflow/
├── extension.yml              # Manifesto da extensão e hooks
├── README.md                  # Documentação de uso
└── scripts/
    ├── git-commit-hook.sh     # Hook pós-etapa para commits convencionais
    ├── git-branch-hook.sh     # Hook pré-etapa para verificação/criação de branch
    └── switch-feature.sh      # Script utilitário para alternar projeto ativo
```

## Funcionamento dos Hooks

### 1. Commits por Etapa (`git-commit-hook.sh`)
Executado automaticamente ao final das etapas do Speckit (`after_specify`, `after_plan`, `after_tasks`, `after_implement`).

- Verifica se existem arquivos alterados/não commitados no Git (`git status --porcelain`).
- Caso haja alterações, sugere uma mensagem de commit no formato Conventional Commits:
  - `specify` -> `docs(docs): Add specification for <feature>`
  - `plan` -> `docs(docs): Add implementation plan for <feature>`
  - `tasks` -> `docs(docs): Add tasks breakdown for <feature>`
  - `implement` -> `feat(software): Implement software tasks for <feature>`
- Pergunta ao usuário se deseja realizar o commit (`[S/n]`).
- Se confirmado, executa `git add -A` e realiza o commit com a mensagem padronizada.

### 2. Verificação de Branch ao Mudar Projeto (`git-branch-hook.sh`)
Executado antes das etapas do Speckit (`before_specify`, `before_plan`, `before_tasks`, `before_implement`).

- Identifica o projeto ativo a partir de `.specify/feature.json`.
- Compara com a branch atual (`git branch --show-current`).
- Se a branch atual for diferente da branch recomendada (`feature/<nome-do-projeto>`), questiona o usuário se deseja criar ou alternar para a branch apropriada.

### 3. Alternar Projeto Ativo Manualmente (`switch-feature.sh`)
Permite alterar o projeto ativo e engatilhar imediatamente a verificação de branch:

```bash
bash .specify/extensions/git-workflow/scripts/switch-feature.sh specs/003-filament-dryer-control-app
```

## Parâmetros e Variáveis de Ambiente

- `--yes` ou `-y`: Aceita automaticamente prompts de commit/branch sem interatividade.
- `--no` ou `-n`: Recusa automaticamente os prompts.
- `SPECIFY_GIT_AUTO_COMMIT=1`: Define confirmação automática de commits no ambiente.
- `SPECIFY_GIT_AUTO_BRANCH=1`: Define confirmação automática de troca de branch no ambiente.
