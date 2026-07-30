#!/usr/bin/env bash
# Wrap Flutter linux release bundle as a portable AppImage-like tarball + optional appimagetool.
set -euo pipefail
APP="$(cd "$(dirname "$0")/../../.." && pwd)"
DIST="$APP/dist"
BUNDLE="$APP/build/linux/x64/release/bundle"
mkdir -p "$DIST"
if [[ ! -d "$BUNDLE" ]]; then
  echo "Missing $BUNDLE — run flutter build linux --release first"
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
# Icon placeholder
printf '' >"$STAGE/philarmony-installer.png" || true
OUT_TAR="$DIST/PhilarmonyInstaller-linux-x64.tar.gz"
tar -C "$STAGE" -czf "$OUT_TAR" .
echo "Wrote $OUT_TAR"
if command -v appimagetool >/dev/null; then
  appimagetool "$STAGE" "$DIST/PhilarmonyInstaller-x86_64.AppImage"
  echo "Wrote $DIST/PhilarmonyInstaller-x86_64.AppImage"
else
  echo "appimagetool not found — tar.gz portable bundle is ready; install appimagetool for .AppImage"
fi
