#!/usr/bin/env bash
# Build .rpm from Flutter linux release using rpmbuild when available.
set -euo pipefail
APP="$(cd "$(dirname "$0")/../../.." && pwd)"
DIST="$APP/dist"
BUNDLE="$APP/build/linux/x64/release/bundle"
SPEC="$APP/packaging/linux/rpm/philarmony-installer.spec"
mkdir -p "$DIST"
if [[ ! -d "$BUNDLE" ]]; then
  echo "Missing $BUNDLE"; exit 1
fi
if ! command -v rpmbuild >/dev/null; then
  # Portable fallback: tar the bundle with Fedora-oriented name
  OUT="$DIST/philarmony-installer-0.1.0-1.x86_64.tar.gz"
  tar -C "$BUNDLE/.." -czf "$OUT" "$(basename "$BUNDLE")"
  echo "rpmbuild not found — wrote portable $OUT"
  echo "On Fedora/RHEL: install rpm-build and re-run; see $SPEC"
  exit 0
fi
TOP="$DIST/rpmbuild"
rm -rf "$TOP"
mkdir -p "$TOP"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
tar -C "$BUNDLE/.." -czf "$TOP/SOURCES/philarmony-installer.tar.gz" "$(basename "$BUNDLE")"
cp "$SPEC" "$TOP/SPECS/"
rpmbuild --define "_topdir $TOP" -bb "$TOP/SPECS/philarmony-installer.spec" || {
  echo "rpmbuild failed — check $SPEC paths; leaving SOURCES in $TOP"
  exit 1
}
find "$TOP/RPMS" -name '*.rpm' -exec cp {} "$DIST/" \;
echo "RPMs copied to $DIST"
