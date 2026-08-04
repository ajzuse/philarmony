#!/usr/bin/env bash
# Build .deb from Flutter linux release bundle using dpkg-deb.
set -euo pipefail
APP="$(cd "$(dirname "$0")/../../.." && pwd)"
DIST="$APP/dist"
BUNDLE="$APP/build/linux/x64/release/bundle"
CTRL="$APP/packaging/linux/deb/control"
mkdir -p "$DIST"
if [[ ! -d "$BUNDLE" ]]; then
  echo "Missing $BUNDLE"; exit 1
fi
if ! command -v dpkg-deb >/dev/null; then
  echo "ERROR: dpkg-deb not found — install dpkg-dev and re-run" >&2
  exit 1
fi
STAGE="$DIST/deb-root"
rm -rf "$STAGE"
mkdir -p "$STAGE/DEBIAN" "$STAGE/usr/lib/philarmony-installer" "$STAGE/usr/bin" \
  "$STAGE/usr/share/applications"
cp -a "$BUNDLE/." "$STAGE/usr/lib/philarmony-installer/"
cat >"$STAGE/usr/bin/philarmony-installer" <<'EOF'
#!/bin/sh
exec /usr/lib/philarmony-installer/esp32_desktop_installer "$@"
EOF
chmod +x "$STAGE/usr/bin/philarmony-installer"
cp "$CTRL" "$STAGE/DEBIAN/control"
# Installed-Size in KiB
size_kb="$(du -sk "$STAGE/usr" | awk '{print $1}')"
echo "Installed-Size: $size_kb" >>"$STAGE/DEBIAN/control"
cat >"$STAGE/usr/share/applications/philarmony-installer.desktop" <<'EOF'
[Desktop Entry]
Name=Philarmony Installer
Exec=philarmony-installer
Type=Application
Categories=Development;Electronics;
EOF
OUT="$DIST/philarmony-installer_0.1.0_amd64.deb"
dpkg-deb --build "$STAGE" "$OUT"
test -f "$OUT"
echo "Wrote $OUT"
