#!/usr/bin/env bash
# switch-feature.sh
# Script to switch active project/feature in Speckit and trigger git branch verification.

set -e

SCRIPT_DIR="$(CDPATH="" cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMMON_SCRIPT="$SCRIPT_DIR/../../../scripts/bash/common.sh"

if [ -f "$COMMON_SCRIPT" ]; then
    source "$COMMON_SCRIPT"
fi

if [ -z "$1" ]; then
    echo "Uso: $0 <caminho_ou_nome_da_feature> [--yes|--no]"
    echo ""
    echo "Exemplo:"
    echo "  $0 specs/003-filament-dryer-control-app"
    echo "  $0 004-esp32-touchscreen-ui"
    exit 1
fi

FEATURE_ARG="$1"
shift

# Resolve repo root
if command -v get_repo_root >/dev/null 2>&1; then
    REPO_ROOT="$(get_repo_root)"
else
    REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
fi

cd "$REPO_ROOT"

# Resolve feature directory path
FEATURE_DIR=""
if [ -d "$FEATURE_ARG" ]; then
    FEATURE_DIR="$FEATURE_ARG"
elif [ -d "specs/$FEATURE_ARG" ]; then
    FEATURE_DIR="specs/$FEATURE_ARG"
else
    # Try finding matching feature dir in specs/
    MATCHING_DIR="$(find specs -maxdepth 1 -type d -name "*$FEATURE_ARG*" 2>/dev/null | head -n 1 || true)"
    if [ -n "$MATCHING_DIR" ]; then
        FEATURE_DIR="$MATCHING_DIR"
    else
        echo "❌ Diretório de feature não encontrado para: $FEATURE_ARG" >&2
        exit 1
    fi
fi

# Update .specify/feature.json using common function or write
if command -v _persist_feature_json >/dev/null 2>&1; then
    _persist_feature_json "$REPO_ROOT" "$FEATURE_DIR"
else
    mkdir -p "$REPO_ROOT/.specify"
    printf '{"feature_directory":"%s"}\n' "$FEATURE_DIR" > "$REPO_ROOT/.specify/feature.json"
fi

echo "🎯 Projeto ativo alterado para: $FEATURE_DIR"

# Execute git branch check hook
"$SCRIPT_DIR/git-branch-hook.sh" --trigger=switch --target-feature="$FEATURE_DIR" "$@"

exit 0
