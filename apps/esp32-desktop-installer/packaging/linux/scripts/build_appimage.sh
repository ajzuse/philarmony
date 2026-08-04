#!/usr/bin/env bash
# Wrap Flutter linux release bundle as AppImage (requires appimagetool).
set -euo pipefail
APP="$(cd "$(dirname "$0")/../../.." && pwd)"
DIST="$APP/dist"
BUNDLE="$APP/build/linux/x64/release/bundle"
mkdir -p "$DIST"
if [[ ! -d "$BUNDLE" ]]; then
  echo "Missing $BUNDLE — run flutter build linux --release first"
  exit 1
fi
if ! command -v appimagetool >/dev/null; then
  echo "ERROR: appimagetool not found — install AppImageKit tools and re-run" >&2
  exit 1
fi
STAGE="$DIST/PhilarmonyInstaller.AppDir"
rm -rf "$STAGE"
mkdir -p "$STAGE/usr/bin" "$STAGE/usr/share/applications" "$STAGE/usr/share/icons/hicolor/256x256/apps"
cp -a "$BUNDLE/." "$STAGE/usr/bin/"
cat >"$STAGE/AppRun" <<'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
exec "$HERE/usr/bin/esp32_desktop_installer" "$@"
EOF
chmod +x "$STAGE/AppRun"
cat >"$STAGE/usr/share/applications/philarmony-installer.desktop" <<'EOF'
[Desktop Entry]
Name=Philarmony Installer
Exec=esp32_desktop_installer
Icon=philarmony-installer
Type=Application
Categories=Development;Electronics;
EOF
cp "$STAGE/usr/share/applications/philarmony-installer.desktop" "$STAGE/philarmony-installer.desktop"
# Icon placeholder (appimagetool accepts empty/missing icon with warning)
printf '' >"$STAGE/philarmony-installer.png" || true
OUT_APPIMAGE="$DIST/PhilarmonyInstaller-x86_64.AppImage"
appimagetool "$STAGE" "$OUT_APPIMAGE"
test -f "$OUT_APPIMAGE"
echo "Wrote $OUT_APPIMAGE"
