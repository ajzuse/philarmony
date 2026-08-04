Name: philarmony-installer
Version: 0.1.0
Release: 1%{?dist}
Summary: Philarmony ESP32 desktop installer
License: GPL-3.0-or-later
URL: https://github.com/ajzuse/philarmony
BuildArch: x86_64
Source0: philarmony-installer.tar.gz

%description
Guided USB firmware installer for Philarmony filament dryer (Fedora/RHEL-family).

%prep
%setup -q -n bundle

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/lib/philarmony-installer
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/applications
cp -a . %{buildroot}/usr/lib/philarmony-installer/
cat > %{buildroot}/usr/bin/philarmony-installer << 'EOF'
#!/bin/sh
exec /usr/lib/philarmony-installer/esp32_desktop_installer "$@"
EOF
chmod 755 %{buildroot}/usr/bin/philarmony-installer
cat > %{buildroot}/usr/share/applications/philarmony-installer.desktop << 'EOF'
[Desktop Entry]
Name=Philarmony Installer
Exec=philarmony-installer
Type=Application
Categories=Development;Electronics;
EOF

%files
/usr/bin/philarmony-installer
/usr/lib/philarmony-installer
/usr/share/applications/philarmony-installer.desktop

%changelog
* Tue Aug 04 2026 Philarmony Contributors - 0.1.0-1
- Initial RPM packaging with Flutter linux release bundle payload
