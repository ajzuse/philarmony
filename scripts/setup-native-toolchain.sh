#!/usr/bin/env bash
# Philarmony — install host toolchain dependencies on macOS/Linux.
# Installs PlatformIO Core (firmware build + native tests) and checks for GNU
# make and a host C++ compiler needed by `pio test -e native`.
# Idempotent: safe to re-run.
# Usage: ./scripts/setup-native-toolchain.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

if command -v python3 >/dev/null 2>&1; then
  PY=python3
elif command -v python >/dev/null 2>&1; then
  PY=python
else
  echo "[error] Python 3 not found. Install it and re-run." >&2
  exit 1
fi

pip_install() {
  "$PY" -m pip install --user "$@" || {
    echo "[warn] pip failed (TLS inspection?); retrying with --trusted-host..." >&2
    "$PY" -m pip install --user \
      --trusted-host pypi.org --trusted-host files.pythonhosted.org "$@"
  }
}

echo "[1/3] Installing PlatformIO Core..."
pip_install -U platformio

echo "[2/3] Checking GNU make..."
if command -v make >/dev/null 2>&1; then
  echo "  make: $(command -v make) ($(make --version 2>/dev/null | head -1))"
else
  echo "  [warn] no GNU make on PATH — install it to use the Makefile targets." >&2
  echo "         macOS: xcode-select --install    Debian/Ubuntu: sudo apt-get install -y make" >&2
fi

echo "[3/3] Checking host C++ compiler..."
if command -v g++ >/dev/null 2>&1; then
  echo "  g++: $(command -v g++)"
elif command -v clang++ >/dev/null 2>&1; then
  echo "  clang++: $(command -v clang++) (PlatformIO native may still expect g++)"
else
  echo "  [warn] no g++/clang++ on PATH — 'pio test -e native' needs a host C++ compiler." >&2
  echo "         macOS:         xcode-select --install" >&2
  echo "         Debian/Ubuntu: sudo apt-get install -y build-essential" >&2
  echo "         Fedora:        sudo dnf install -y gcc-c++" >&2
fi

SCRIPTS="$("$PY" -c 'import sysconfig;print(sysconfig.get_path("scripts","posix_user"))' 2>/dev/null || true)"
echo
echo "Done. Ensure the pip user-scripts dir is on PATH:"
echo "  ${SCRIPTS:-$HOME/.local/bin}"
echo "Then run:  pio test -e native   |   pio run -e esp32devkitc"
