# Philarmony Desktop Installer

## Download

Get artifacts from [GitHub Releases](https://github.com/ajzuse/philarmony/releases):

| OS | File | Notes |
|----|------|-------|
| Windows | `.msix` (or Inno `.exe`) | MVP may be **unsigned** — SmartScreen may warn |
| macOS | `.dmg` | Public release **must** be signed + notarized |
| Linux | `.AppImage`, `.deb`, `.rpm` | MVP may be unsigned; use local `dnf`/`apt` |

### Linux

```bash
chmod +x PhilarmonyInstaller-*-linux-x64.AppImage && ./PhilarmonyInstaller-*-linux-x64.AppImage
sudo apt install ./PhilarmonyInstaller-*-linux-x64.deb
sudo dnf install ./PhilarmonyInstaller-*-linux-x64.rpm
```

## Development

Use **FVM** (Flutter **3.44.0**): `fvm use 3.44.0`, then `fvm flutter …`.  
See `apps/esp32-desktop-installer/packaging/README.md` and `make help`.
