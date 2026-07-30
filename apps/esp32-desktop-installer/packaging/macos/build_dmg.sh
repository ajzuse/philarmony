#!/usr/bin/env bash
# Create a DMG from the macOS .app (unsigned OK for local; notarize for public).
set -euo pipefail
APP_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
DIST="$APP_DIR/dist"
APP_BUNDLE="$APP_DIR/build/macos/Build/Products/Release/esp32_desktop_installer.app"
# Flutter may nest under different product name
if [[ ! -d "$APP_BUNDLE" ]]; then
  APP_BUNDLE="$(find "$APP_DIR/build/macos" -name '*.app' -type d | head -1 || true)"
fi
mkdir -p "$DIST"
if [[ -z "${APP_BUNDLE}" || ! -d "$APP_BUNDLE" ]]; then
  echo "Missing macOS .app — run: fvm flutter build macos --release"
  exit 1
fi
VOL="PhilarmonyInstaller"
STAGE="$DIST/dmg-stage"
rm -rf "$STAGE"
mkdir -p "$STAGE"
cp -R "$APP_BUNDLE" "$STAGE/"
ln -sf /Applications "$STAGE/Applications"
OUT="$DIST/PhilarmonyInstaller-macos.dmg"
rm -f "$OUT"
hdiutil create -volname "$VOL" -srcfolder "$STAGE" -ov -format UDZO "$OUT"
echo "Wrote $OUT"
cat <<'NOTE'
Public release checklist (REQUIRED):
1. codesign --deep --force --options runtime --sign "Developer ID Application: …" *.app
2. xcrun notarytool submit "$OUT" --apple-id … --team-id … --password … --wait
3. xcrun stapler staple "$OUT"
Store secrets in CI only (APPLE_ID, APPLE_APP_SPECIFIC_PASSWORD, TEAM_ID, CERTIFICATES_P12).
NOTE
