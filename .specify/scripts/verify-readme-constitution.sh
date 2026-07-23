#!/usr/bin/env bash
# ============================================================================
# Philarmony - Verificação de Conformidade README-Constituição
# ============================================================================

set -euo pipefail

PROJECT_ROOT="/Users/ajzuse/projects/secadora"
README_FILE="$PROJECT_ROOT/README.md"
CONSTITUTION_FILE="$PROJECT_ROOT/.specify/memory/constitution.md"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${BLUE}[VERIFY]${NC} $1"; }
pass() { echo -e "${GREEN}[PASS]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
fail() { echo -e "${RED}[FAIL]${NC} $1"; }

ERRORS=0

check_constitution_version() {
    log "Verificando versão da Constituição no README..."
    local constitution_version=$(grep -E '^\*\*Version\*\*:' "$CONSTITUTION_FILE" | sed 's/.*Version\*\*: *//' | sed 's/ |.*//')
    local readme_version=$(grep -o 'Constitution-v[0-9.]\+' "$README_FILE" | head -1 | sed 's/Constitution-v//')
    
    if [[ "$constitution_version" == "$readme_version" ]]; then
        pass "Versão Constituição: v$constitution_version (README: v$readme_version)"
    else
        fail "Versão divergente: Constituição v$constitution_version vs README v$readme_version"
        ((ERRORS++))
    fi
}

check_principles_documented() {
    log "Verificando se princípios constitucionais estão referenciados..."
    local principles=(
        "Orientação a Objetos"
        "Desempenho Máximo"
        "Failsafe"
        "Workspace de Dependências"
        "Teste Automatizado"
        "Documentação Sincronizada"
    )
    
    for principle in "${principles[@]}"; do
        if grep -qi "$principle" "$README_FILE"; then
            pass "Princípio '$principle' referenciado no README"
        else
            warn "Princípio '$principle' NÃO encontrado no README"
        fi
    done
}

check_governance_refs() {
    log "Verificando referências de governança..."
    local refs=(
        "Conventional Commits"
        "GPLv3"
        "README.*sincronizado"
        "hooks"
    )
    
    for ref in "${refs[@]}"; do
        if grep -qi "$ref" "$README_FILE"; then
            pass "Referência '$ref' presente"
        else
            warn "Referência '$ref' NÃO encontrada"
        fi
    done
}

check_specs_status() {
    log "Verificando status das specs no README..."
    local spec_dirs=$(find "$PROJECT_ROOT/specs" -maxdepth 1 -type d -name "0*" 2>/dev/null | wc -l)
    local specs_in_readme=$(grep -c '| 0[0-9] ' "$README_FILE" || echo 0)
    
    if [[ $specs_in_readme -ge $spec_dirs ]]; then
        pass "Todas as $spec_dirs specs referenciadas no README ($specs_in_readme linhas de tabela)"
    else
        warn "Specs no filesystem: $spec_dirs, no README: $specs_in_readme"
    fi
}

check_roadmap_current() {
    log "Verificando roadmap atualizado..."
    if grep -q "Specification.*CONCLUÍDO" "$README_FILE"; then
        pass "Roadmap mostra Specification como CONCLUÍDO"
    else
        warn "Roadmap pode estar desatualizado"
    fi
}

check_license_header() {
    log "Verificando header GPLv3..."
    if grep -q "GNU General Public License v3.0" "$README_FILE"; then
        pass "Licença GPLv3 documentada"
    else
        fail "Licença GPLv3 NÃO encontrada no README"
        ((ERRORS++))
    fi
}

check_bilingual() {
    log "Verificando bilinguismo PT-BR/EN-US..."
    if grep -q "PT-BR" "$README_FILE" && grep -q "EN-US" "$README_FILE"; then
        pass "Bilinguismo PT-BR/EN-US referenciado"
    else
        warn "Bilinguismo não claramente documentado"
    fi
}

# ============================================================================
# MAIN
# ============================================================================

log "=== Verificação README vs Constituição ==="

check_constitution_version
check_principles_documented
check_governance_refs
check_specs_status
check_roadmap_current
check_license_header
check_bilingual

echo ""
if [[ $ERRORS -eq 0 ]]; then
    echo -e "${GREEN}✅ Verificação concluída: 0 erros críticos${NC}"
    exit 0
else
    echo -e "${RED}❌ Verificação falhou: $ERRORS erro(s) crítico(s)${NC}"
    exit 1
fi