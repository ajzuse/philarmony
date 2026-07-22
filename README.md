# Philarmony

## License

This project is licensed under the GNU General Public License v3.0

Copyright © 2024-2026 ESP32 DIY Hardware Team

Everyone is permitted to copy and distribute verbatim copies
of this license document, but changing it is not allowed.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see
<https://www.gnu.org/licenses/>.

## O que é o Philarmony?

Philarmony é uma plataforma de workspace opensource para subprojetos baseados em OOP, focado em desempenho máximo para hardware ESP32. É uma caixa de secagem de filamento DIY controlada por ESP32, projetada para ser failsafe com salva guardas para abortar a execução caso os parametros saiam de controle, sem comprometer o hardware e sem travamentos.

## Principais Características

- **Orientação a Objetos**: Arquitetura baseada em conceitos de OOP para melhor manutenibilidade e escalabilidade
- **Focado em Desempenho**: Otimizado para desempenho máximo no hardware ESP32
- **Failsafe**: Implementação de seguridad protectora que impede diretamente danos causados por parâmetros fora de controle
- **Workspace de Dependências**: Organizado e compartilhado através de configurações para o Visual Studio Code, assegurando coerência entre várias máquinas
- **Automação** de teste do workflow pelos testes de fluxo e processo de desenvolvimento de TDD
- **Documentação**: Contendo material PT-BR e EN-US para documentação completa e internacionalizada
- **Constituição**: Regras de projeto institucionalizadas no arquivo .specify/memory/constitution.md

## Configuração do Desenvolvimento

### Ferramentas Principais

- **IDE**: Visual Studio Code com workspace compartilhado
- **Controle de Versão**: Git com Conventional Commits
- **Linter**: commitlint para mensagens de commits válidas
- **Teste**: Pipeline de teste automatizado para testes de fluxo

### Fluxo de Trabalho

1. **Branch Strategy**: Crie uma nova branch (`feature/xxx`) para cada bloco de tarefas
2. **Commits**: Faça commits entre tarefas após a validação dos testes
3. **Pull Requests**: Abra um pull request ao final da execução de cada bloco
4. **Convenções de Commit**: Siga as convenções de commit convencionais para mensagens de commits

## Estrutura do Projeto

```
workspace_root/
├── .specify/
│   ├── memory/
│   │   └── constitution.md
│   ├── templates/
│   │   ├── plan-template.md
│   │   ├── spec-template.md
│   │   └── tasks-template.md
│   ├── scripts/
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
├── README.md
└── LICENSE
```

## Habilitando o Workspace

```bash
# Abra o projeto no VS Code
# A configuração compartilhada do workspace será aplicada automaticamente
# Todas as extensões e definições no .vscode/ estarão disponíveis
```

## Testes

Todo o desenvolvimento deve seguir as práticas de Test-First (TDD) sempre que possível. Se o TDD não for possível, apenas os testes de fluxo completo são permitidos.

## Changelog

Todas as alterações de conteúdop devem ser documentadas no CHANGELOG.md, seguindo as convenções de commit convencionais.

## Contribuição

Para contribuir, por favor siga estas instruções:

1. Faça um fork deste repositório
2. Crie um branch para seu recurso (`git checkout -b feature/xxx`)
3. Faça commit de suas alterações (`git commit -a`)
4. Faça push para o branch (`git push origin feature/xxx`)
5. Abra um Pull Request

## Documentação

Consulte a documentação em `docs/` para guias detalhados sobre a configuração, uso, e desenvolvimento do projeto.

## Segurança

Todos os commits devem seguir as convenções convencionais de commit para garantir um histórico rastreável e de qualidade.
