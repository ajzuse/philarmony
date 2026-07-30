; Optional Inno Setup script for Windows portable installer (US5 / T055).
; Requires Inno Setup 6+: https://jrsoftware.org/isinfo.php
; Build Flutter windows release first, then compile this script.

#define MyAppName "Philarmony Installer"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "Philarmony"
#define MyAppExeName "esp32_desktop_installer.exe"

[Setup]
AppId={{A7C2E5F1-9B44-4D3A-9E2C-PhilarmonyInst}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Philarmony\Installer
DefaultGroupName={#MyAppName}
OutputDir=..\..\..\dist
OutputBaseFilename=PhilarmonyInstaller-Setup-{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
ArchitecturesInstallIn64BitMode=x64compatible
; Unsigned OK for MVP — document SmartScreen warnings in docs.

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Files]
; Adjust path after `flutter build windows --release`
Source: "..\..\..\build\windows\x64\runner\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent
