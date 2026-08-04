#!/usr/bin/env bash
# Build .rpm from Flutter linux release using rpmbuild (hard-fail if missing).
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
  echo "ERROR: rpmbuild not found — install rpm-build and re-run; see $SPEC" >&2
  exit 1
fi
TOP="$DIST/rpmbuild"
rm -rf "$TOP"
mkdir -p "$TOP"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
# Source tarball must expand to a top-level "bundle/" directory (%setup -n bundle)
tar -C "$BUNDLE/.." -czf "$TOP/SOURCES/philarmony-installer.tar.gz" "$(basename "$BUNDLE")"
cp "$SPEC" "$TOP/SPECS/"
rpmbuild --define "_topdir $TOP" -bb "$TOP/SPECS/philarmony-installer.spec"
find "$TOP/RPMS" -name '*.rpm' -exec cp {} "$DIST/" \;
test -n "$(find "$DIST" -maxdepth 1 -name '*.rpm' | head -1)"
echo "RPMs copied to $DIST"
