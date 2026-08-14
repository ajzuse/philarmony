#!/usr/bin/env bash
# Wrap Flutter linux release bundle as AppImage for Philarmony Control.
set -euo pipefail
APP_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DIST="$APP_DIR/dist"
BUNDLE="$(find "$APP_DIR/build/linux" -path '*/release/bundle' -type d 2>/dev/null | head -1 || true)"
mkdir -p "$DIST"
if [[ -z "$BUNDLE" || ! -d "$BUNDLE" ]]; then
  echo "Missing linux bundle — run: flutter build linux --release first"
  exit 1
fi
if ! command -v appimagetool >/dev/null; then
  echo "ERROR: appimagetool not found — install AppImageKit tools and re-run" >&2
  exit 1
fi
STAGE="$DIST/PhilarmonyControl.AppDir"
rm -rf "$STAGE"
mkdir -p "$STAGE/usr/bin" "$STAGE/usr/share/applications" "$STAGE/usr/share/icons/hicolor/256x256/apps"
cp -a "$BUNDLE/." "$STAGE/usr/bin/"
cat >"$STAGE/AppRun" <<'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
exec "$HERE/usr/bin/filament_dryer_control" "$@"
EOF
chmod +x "$STAGE/AppRun"
cat >"$STAGE/usr/share/applications/philarmony-control.desktop" <<'EOF'
[Desktop Entry]
Name=Philarmony Control
Exec=filament_dryer_control
Icon=philarmony-control
Type=Application
Categories=Utility;Electronics;
EOF
cp "$STAGE/usr/share/applications/philarmony-control.desktop" "$STAGE/philarmony-control.desktop"
printf '' >"$STAGE/philarmony-control.png" || true
OUT_APPIMAGE="$DIST/PhilarmonyControl-x86_64.AppImage"
appimagetool "$STAGE" "$OUT_APPIMAGE"
test -f "$OUT_APPIMAGE"
echo "Wrote $OUT_APPIMAGE"
