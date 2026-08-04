#!/usr/bin/env bash
# Notarize and staple a macOS DMG for public release (FR-012).
# The .app MUST already be codesigned before build_dmg.sh (see CI / build_dmg.sh).
set -euo pipefail

DMG="${1:-}"
if [[ -z "$DMG" || ! -f "$DMG" ]]; then
  echo "Usage: notarize.sh <path-to.dmg>" >&2
  exit 1
fi

: "${APPLE_ID:?APPLE_ID required}"
: "${APPLE_TEAM_ID:?APPLE_TEAM_ID required}"
: "${APPLE_APP_SPECIFIC_PASSWORD:?APPLE_APP_SPECIFIC_PASSWORD required}"

echo "Submitting $DMG to notarytool…"
xcrun notarytool submit "$DMG" \
  --apple-id "$APPLE_ID" \
  --team-id "$APPLE_TEAM_ID" \
  --password "$APPLE_APP_SPECIFIC_PASSWORD" \
  --wait

echo "Stapling…"
xcrun stapler staple "$DMG"
echo "Notarization complete: $DMG"
