#!/usr/bin/env bash
# Create a DMG from the macOS .app for Philarmony Control.
set -euo pipefail
APP_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DIST="$APP_DIR/dist"
APP_BUNDLE="$APP_DIR/build/macos/Build/Products/Release/filament_dryer_control.app"
if [[ ! -d "$APP_BUNDLE" ]]; then
  APP_BUNDLE="$(find "$APP_DIR/build/macos" -name '*.app' -type d | head -1 || true)"
fi
mkdir -p "$DIST"
if [[ -z "${APP_BUNDLE}" || ! -d "$APP_BUNDLE" ]]; then
  echo "Missing macOS .app — run: flutter build macos --release"
  exit 1
fi

if [[ -n "${APPLE_SIGNING_IDENTITY:-}" ]]; then
  echo "codesign (pre-DMG): $APP_BUNDLE"
  codesign --deep --force --options runtime \
    --sign "$APPLE_SIGNING_IDENTITY" "$APP_BUNDLE"
fi

VOL="PhilarmonyControl"
STAGE="$DIST/dmg-stage"
rm -rf "$STAGE"
mkdir -p "$STAGE"
cp -R "$APP_BUNDLE" "$STAGE/"
ln -sf /Applications "$STAGE/Applications"
OUT="$DIST/PhilarmonyControl-macos.dmg"
rm -f "$OUT"
hdiutil create -volname "$VOL" -srcfolder "$STAGE" -ov -format UDZO "$OUT"
echo "Wrote $OUT"
