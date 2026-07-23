#!/usr/bin/env bash
# git-commit-hook.sh
# Speckit extension hook for prompting and creating Conventional Commits after each SDD stage.

set -e

SCRIPT_DIR="$(CDPATH="" cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMMON_SCRIPT="$SCRIPT_DIR/../../../scripts/bash/common.sh"

if [ -f "$COMMON_SCRIPT" ]; then
    source "$COMMON_SCRIPT"
fi

STAGE="specify"
AUTO_YES=false
AUTO_NO=false
CUSTOM_MSG=""

while [ $# -gt 0 ]; do
    case "$1" in
        --stage=*)
            STAGE="${1#*=}"
            ;;
        --stage)
            STAGE="$2"
            shift
            ;;
        --yes|-y)
            AUTO_YES=true
            ;;
        --no|-n)
            AUTO_NO=true
            ;;
        --message=*|--msg=*)
            CUSTOM_MSG="${1#*=}"
            ;;
        --message|--msg|-m)
            CUSTOM_MSG="$2"
            shift
            ;;
        *)
            ;;
    esac
    shift
done

# Resolve repo root
if command -v get_repo_root >/dev/null 2>&1; then
    REPO_ROOT="$(get_repo_root)"
else
    REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
fi

cd "$REPO_ROOT"

# Check if there are any uncommitted changes (staged or unstaged)
UNCOMMITTED_CHANGES="$(git status --porcelain 2>/dev/null || true)"
if [ -z "$UNCOMMITTED_CHANGES" ]; then
    echo "ℹ️ [git-workflow] Nenhum arquivo alterado ou não commitado na etapa '$STAGE'. Commit não necessário."
    exit 0
fi

# Resolve feature name
FEATURE_NAME=""
if [ -f "$REPO_ROOT/.specify/feature.json" ]; then
    if command -v read_feature_json_feature_directory >/dev/null 2>&1; then
        RAW_FD="$(read_feature_json_feature_directory "$REPO_ROOT")"
        FEATURE_NAME="$(basename "$RAW_FD")"
    fi
fi

if [ -z "$FEATURE_NAME" ] && [ -n "${SPECIFY_FEATURE:-}" ]; then
    FEATURE_NAME="$SPECIFY_FEATURE"
fi

if [ -z "$FEATURE_NAME" ]; then
    FEATURE_NAME="projeto"
fi

# Clean feature name for commit scope / message (e.g. 003-filament-dryer-control-app -> filament-dryer)
CLEAN_FEATURE="$(echo "$FEATURE_NAME" | sed -E 's/^[0-9]+-//')"

# Helper function to capitalize first letter for sentence-case subject
to_sentence_case() {
    local text="$1"
    if [ -z "$text" ]; then
        echo ""
        return
    fi
    local first="$(echo "${text:0:1}" | tr '[:lower:]' '[:upper:]')"
    local rest="${text:1}"
    echo "${first}${rest}"
}

# Generate conventional commit message
if [ -n "$CUSTOM_MSG" ]; then
    COMMIT_MSG="$CUSTOM_MSG"
else
    case "$STAGE" in
        specify)
            SUBJ="Add specification for ${CLEAN_FEATURE}"
            SUBJ="$(to_sentence_case "$SUBJ")"
            COMMIT_MSG="docs(docs): ${SUBJ}"
            ;;
        plan)
            SUBJ="Add implementation plan for ${CLEAN_FEATURE}"
            SUBJ="$(to_sentence_case "$SUBJ")"
            COMMIT_MSG="docs(docs): ${SUBJ}"
            ;;
        tasks)
            SUBJ="Add tasks breakdown for ${CLEAN_FEATURE}"
            SUBJ="$(to_sentence_case "$SUBJ")"
            COMMIT_MSG="docs(docs): ${SUBJ}"
            ;;
        implement)
            SUBJ="Implement software tasks for ${CLEAN_FEATURE}"
            SUBJ="$(to_sentence_case "$SUBJ")"
            COMMIT_MSG="feat(software): ${SUBJ}"
            ;;
        constitution)
            COMMIT_MSG="docs(docs): Update project constitution"
            ;;
        *)
            SUBJ="Complete ${STAGE} stage for ${CLEAN_FEATURE}"
            SUBJ="$(to_sentence_case "$SUBJ")"
            COMMIT_MSG="chore(chore): ${SUBJ}"
            ;;
    esac
fi

# Enforce header max length 72 chars per commitlint configuration
if [ ${#COMMIT_MSG} -gt 72 ]; then
    COMMIT_MSG="${COMMIT_MSG:0:69}..."
fi

echo ""
echo "=================================================="
echo "📌 Speckit Git Workflow - Validação de Commit"
echo "--------------------------------------------------"
echo "Etapa concluída : $STAGE"
echo "Projeto ativo   : $FEATURE_NAME"
echo "Commit sugerido : $COMMIT_MSG"
echo "=================================================="
echo ""

DO_COMMIT=false

if [ "$AUTO_YES" = true ]; then
    DO_COMMIT=true
elif [ "$AUTO_NO" = true ]; then
    DO_COMMIT=false
elif [ -n "${SPECIFY_GIT_AUTO_COMMIT:-}" ] && [ "${SPECIFY_GIT_AUTO_COMMIT}" != "0" ] && [ "${SPECIFY_GIT_AUTO_COMMIT}" != "false" ]; then
    DO_COMMIT=true
elif [ -t 0 ] || [ -e /dev/tty ]; then
    # Try reading from terminal if available
    TTY_DEV="/dev/tty"
    if [ ! -e "$TTY_DEV" ]; then
        TTY_DEV="&1"
    fi
    
    printf "Deseja realizar o commit com a mensagem acima? [S/n]: "
    if [ -e /dev/tty ]; then
        read -r ANSWER < /dev/tty || ANSWER="s"
    else
        read -r ANSWER || ANSWER="s"
    fi

    case "$(echo "$ANSWER" | tr '[:upper:]' '[:lower:]')" in
        s|sim|y|yes|"")
            DO_COMMIT=true
            ;;
        *)
            DO_COMMIT=false
            ;;
    esac
else
    # Non-interactive mode (e.g. subagent or script execution without tty)
    echo "⚠️ Ambientes não-interativo detectado sem TTY direto."
    echo "Para auto-commit em scripts, utilize '--yes' ou exporte SPECIFY_GIT_AUTO_COMMIT=1."
    printf "Deseja confirmar o commit '$COMMIT_MSG'? [S/n]: "
    if read -t 10 -r ANSWER; then
        case "$(echo "$ANSWER" | tr '[:upper:]' '[:lower:]')" in
            s|sim|y|yes|"")
                DO_COMMIT=true
                ;;
            *)
                DO_COMMIT=false
                ;;
        esac
    else
        echo ""
        echo "ℹ️ Tempo limite esgotado. Commit não realizado automaticamente."
        DO_COMMIT=false
    fi
fi

if [ "$DO_COMMIT" = true ]; then
    echo "🚀 Realizando 'git add -A' e commit..."
    git add -A
    git commit -m "$COMMIT_MSG"
    echo "✅ Commit realizado com sucesso!"
    echo "   Message: $COMMIT_MSG"
else
    echo "ℹ️ Commit cancelado pelo usuário. As alterações permanecem no working directory."
fi

exit 0
