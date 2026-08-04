#!/usr/bin/env bash
# Create a DMG from the macOS .app.
# Public releases: codesign the .app BEFORE invoking this script, then notarize the DMG.
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

# Optional local codesign when identity is provided and app not yet signed
if [[ -n "${APPLE_SIGNING_IDENTITY:-}" ]]; then
  echo "codesign (pre-DMG): $APP_BUNDLE"
  codesign --deep --force --options runtime \
    --sign "$APPLE_SIGNING_IDENTITY" "$APP_BUNDLE"
  codesign --verify --deep --strict "$APP_BUNDLE" || {
    echo "codesign verify failed" >&2
    exit 1
  }
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
   (must happen BEFORE this DMG is created — CI does this; or set APPLE_SIGNING_IDENTITY)
2. xcrun notarytool submit "$OUT" --apple-id … --team-id … --password … --wait
3. xcrun stapler staple "$OUT"
Store secrets in CI only (APPLE_ID, APPLE_APP_SPECIFIC_PASSWORD, TEAM_ID, APPLE_SIGNING_IDENTITY).
NOTE
