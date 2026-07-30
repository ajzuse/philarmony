# Instalador Desktop Philarmony

## Download

Baixe o artefato do [GitHub Releases](https://github.com/ajzuse/philarmony/releases):

| SO | Arquivo | Observação |
|----|---------|------------|
| Windows | `.msix` (ou `.exe` Inno) | MVP pode ser **não assinado** — SmartScreen pode avisar |
| macOS | `.dmg` | Release público **deve** ser assinado + notarizado |
| Linux | `.AppImage`, `.deb`, `.rpm` | MVP pode ser não assinado; use `dnf`/`apt` local |

### Linux

```bash
# AppImage
chmod +x PhilarmonyInstaller-*-linux-x64.AppImage && ./PhilarmonyInstaller-*-linux-x64.AppImage

# Debian/Ubuntu
sudo apt install ./PhilarmonyInstaller-*-linux-x64.deb

# Fedora / RHEL / Rocky / Alma
sudo dnf install ./PhilarmonyInstaller-*-linux-x64.rpm
```

## Desenvolvimento

Use **FVM** (Flutter **3.44.0**): `fvm use 3.44.0`, depois `fvm flutter …`.  
Ver `apps/esp32-desktop-installer/packaging/README.md` e `make help`.
