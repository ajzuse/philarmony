#!/usr/bin/env bash
# git-branch-hook.sh
# Speckit extension hook for prompting and creating/switching git branch when active feature/project changes.

set -e

SCRIPT_DIR="$(CDPATH="" cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMMON_SCRIPT="$SCRIPT_DIR/../../../scripts/bash/common.sh"

if [ -f "$COMMON_SCRIPT" ]; then
    source "$COMMON_SCRIPT"
fi

TRIGGER="change"
TARGET_FEATURE_PARAM=""
AUTO_YES=false
AUTO_NO=false

while [ $# -gt 0 ]; do
    case "$1" in
        --trigger=*)
            TRIGGER="${1#*=}"
            ;;
        --trigger)
            TRIGGER="$2"
            shift
            ;;
        --target-feature=*|--feature=*)
            TARGET_FEATURE_PARAM="${1#*=}"
            ;;
        --target-feature|--feature)
            TARGET_FEATURE_PARAM="$2"
            shift
            ;;
        --yes|-y)
            AUTO_YES=true
            ;;
        --no|-n)
            AUTO_NO=true
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

# Resolve target feature name
FEATURE_DIR_VAL=""
if [ -n "$TARGET_FEATURE_PARAM" ]; then
    FEATURE_DIR_VAL="$TARGET_FEATURE_PARAM"
elif [ -f "$REPO_ROOT/.specify/feature.json" ]; then
    if command -v read_feature_json_feature_directory >/dev/null 2>&1; then
        FEATURE_DIR_VAL="$(read_feature_json_feature_directory "$REPO_ROOT")"
    fi
fi

if [ -z "$FEATURE_DIR_VAL" ] && [ -n "${SPECIFY_FEATURE_DIRECTORY:-}" ]; then
    FEATURE_DIR_VAL="$SPECIFY_FEATURE_DIRECTORY"
fi

if [ -z "$FEATURE_DIR_VAL" ] && [ -n "${SPECIFY_FEATURE:-}" ]; then
    FEATURE_DIR_VAL="$SPECIFY_FEATURE"
fi

if [ -z "$FEATURE_DIR_VAL" ]; then
    # No active feature set, nothing to do
    exit 0
fi

FEATURE_NAME="$(basename "$FEATURE_DIR_VAL")"

# Naming convention for branches: feature/<feature_name> or <feature_name>
TARGET_BRANCH="feature/${FEATURE_NAME}"
ALTERNATIVE_BRANCH="${FEATURE_NAME}"

# Get current git branch
CURRENT_BRANCH="$(git branch --show-current 2>/dev/null || git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "")"

# Check if we are already on the expected branch
if [ "$CURRENT_BRANCH" = "$TARGET_BRANCH" ] || [ "$CURRENT_BRANCH" = "$ALTERNATIVE_BRANCH" ]; then
    echo "ℹ️ [git-workflow] Já está na branch correspondente ao projeto ativo ('$CURRENT_BRANCH')."
    exit 0
fi

echo ""
echo "=================================================="
echo "🔀 Speckit Git Workflow - Mudança de Projeto Ativo"
echo "--------------------------------------------------"
echo "Projeto ativo  : $FEATURE_NAME"
echo "Branch atual   : ${CURRENT_BRANCH:-'(detalhado/sem branch)'}"
echo "Branch sugerida: $TARGET_BRANCH"
echo "=================================================="
echo ""

DO_SWITCH=false

if [ "$AUTO_YES" = true ]; then
    DO_SWITCH=true
elif [ "$AUTO_NO" = true ]; then
    DO_SWITCH=false
elif [ -n "${SPECIFY_GIT_AUTO_BRANCH:-}" ] && [ "${SPECIFY_GIT_AUTO_BRANCH}" != "0" ] && [ "${SPECIFY_GIT_AUTO_BRANCH}" != "false" ]; then
    DO_SWITCH=true
elif [ -t 0 ] || [ -e /dev/tty ]; then
    printf "Deseja criar/alternar para a branch '$TARGET_BRANCH'? [S/n]: "
    if [ -e /dev/tty ]; then
        read -r ANSWER < /dev/tty || ANSWER="s"
    else
        read -r ANSWER || ANSWER="s"
    fi

    case "$(echo "$ANSWER" | tr '[:upper:]' '[:lower:]')" in
        s|sim|y|yes|"")
            DO_SWITCH=true
            ;;
        *)
            DO_SWITCH=false
            ;;
    esac
else
    # Non-interactive mode
    echo "⚠️ Ambiente não-interativo detectado."
    echo "Para auto-branch em scripts, utilize '--yes' ou exporte SPECIFY_GIT_AUTO_BRANCH=1."
    printf "Deseja alternar para a branch '$TARGET_BRANCH'? [S/n]: "
    if read -t 10 -r ANSWER; then
        case "$(echo "$ANSWER" | tr '[:upper:]' '[:lower:]')" in
            s|sim|y|yes|"")
                DO_SWITCH=true
                ;;
            *)
                DO_SWITCH=false
                ;;
        esac
    else
        echo ""
        echo "ℹ️ Tempo limite esgotado. Mantendo branch atual."
        DO_SWITCH=false
    fi
fi

if [ "$DO_SWITCH" = true ]; then
    # Check if target branch or alternative branch exists
    if git show-ref --verify --quiet "refs/heads/$TARGET_BRANCH" 2>/dev/null; then
        echo "🚀 Alternando para a branch existente '$TARGET_BRANCH'..."
        git checkout "$TARGET_BRANCH"
        echo "✅ Branch alternada com sucesso: $TARGET_BRANCH"
    elif git show-ref --verify --quiet "refs/heads/$ALTERNATIVE_BRANCH" 2>/dev/null; then
        echo "🚀 Alternando para a branch existente '$ALTERNATIVE_BRANCH'..."
        git checkout "$ALTERNATIVE_BRANCH"
        echo "✅ Branch alternada com sucesso: $ALTERNATIVE_BRANCH"
    else
        echo "🚀 Criando e alternando para a nova branch '$TARGET_BRANCH'..."
        git checkout -b "$TARGET_BRANCH"
        echo "✅ Nova branch criada e ativa: $TARGET_BRANCH"
    fi
else
    echo "ℹ️ Mantendo a branch atual ('$CURRENT_BRANCH')."
fi

exit 0
