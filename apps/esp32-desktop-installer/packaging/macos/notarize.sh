#!/usr/bin/env bash
# Notarize and staple a macOS DMG for public release (FR-012).
set -euo pipefail

DMG="${1:-}"
if [[ -z "$DMG" || ! -f "$DMG" ]]; then
  echo "Usage: notarize.sh <path-to.dmg>" >&2
  exit 1
fi

: "${APPLE_ID:?APPLE_ID required}"
: "${APPLE_TEAM_ID:?APPLE_TEAM_ID required}"
: "${APPLE_APP_SPECIFIC_PASSWORD:?APPLE_APP_SPECIFIC_PASSWORD required}"

APP_BUNDLE="$(find "$(dirname "$DMG")/../build/macos" -name '*.app' -type d 2>/dev/null | head -1 || true)"
if [[ -n "${APPLE_SIGNING_IDENTITY:-}" && -n "${APP_BUNDLE}" && -d "${APP_BUNDLE}" ]]; then
  echo "codesign app bundle: $APP_BUNDLE"
  codesign --deep --force --options runtime \
    --sign "$APPLE_SIGNING_IDENTITY" "$APP_BUNDLE"
fi

echo "Submitting $DMG to notarytool…"
xcrun notarytool submit "$DMG" \
  --apple-id "$APPLE_ID" \
  --team-id "$APPLE_TEAM_ID" \
  --password "$APPLE_APP_SPECIFIC_PASSWORD" \
  --wait

echo "Stapling…"
xcrun stapler staple "$DMG"
echo "Notarization complete: $DMG"
