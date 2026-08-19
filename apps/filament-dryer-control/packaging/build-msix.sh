#!/usr/bin/env bash
# Build MSIX for Philarmony Control (Windows host).
set -euo pipefail
APP_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DIST="$APP_DIR/dist"
mkdir -p "$DIST"
cd "$APP_DIR"
if [[ ! -d build/windows/x64/runner/Release ]]; then
  echo "Missing Windows release — run: flutter build windows --release"
  exit 1
fi
dart run msix:create
msix="$(find "$APP_DIR" -maxdepth 2 -name '*.msix' -type f | head -1 || true)"
if [[ -z "$msix" ]]; then
  echo "MSIX not found after msix:create" >&2
  exit 1
fi
cp "$msix" "$DIST/PhilarmonyControl-windows.msix"
echo "Wrote $DIST/PhilarmonyControl-windows.msix"
