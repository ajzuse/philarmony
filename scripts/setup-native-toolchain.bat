@echo off
setlocal EnableExtensions
REM Philarmony — install host toolchain dependencies on Windows.
REM Installs PlatformIO Core (firmware build + native tests), GNU make, and —
REM when no host GNU compiler is present — the zig-based gcc/g++/ar/ranlib
REM shims so that `pio test -e native` works without MinGW/MSYS2.
REM Idempotent: safe to re-run. Run from Git Bash (`make setup`) or cmd.

set "ROOT=%~dp0.."
set "SHIM=%ROOT%\scripts\native-toolchain\zig-shim.c"
set "TOOLDIR=%LOCALAPPDATA%\philarmony-toolchain\bin"
set "MAKE_URL=https://downloads.sourceforge.net/project/ezwinports/make-4.4.1-without-guile-w32-bin.zip"

REM --- locate python ---
set "PY="
where py >nul 2>nul && set "PY=py -3"
if not defined PY ( where python >nul 2>nul && set "PY=python" )
if not defined PY (
  echo [error] Python 3 not found on PATH. Install from https://www.python.org/downloads/ and re-run.
  exit /b 1
)

echo [1/6] Installing PlatformIO Core...
%PY% -m pip install --user -U platformio
if errorlevel 1 (
  echo [warn] pip failed ^(TLS inspection?^); retrying with --trusted-host...
  %PY% -m pip install --user -U platformio --trusted-host pypi.org --trusted-host files.pythonhosted.org
  if errorlevel 1 exit /b 1
)

echo [2/6] Installing zig ^(host C/C++ compiler^)...
%PY% -m pip install --user -U ziglang
if errorlevel 1 (
  echo [warn] pip failed ^(TLS inspection?^); retrying with --trusted-host...
  %PY% -m pip install --user -U ziglang --trusted-host pypi.org --trusted-host files.pythonhosted.org
  if errorlevel 1 exit /b 1
)

echo [3/6] Locating zig and pip scripts...
for /f "usebackq delims=" %%i in (`%PY% -c "import os,ziglang;print(os.path.dirname(ziglang.__file__))"`) do set "ZIGDIR=%%i"
for /f "usebackq delims=" %%i in (`%PY% -c "import sysconfig;print(sysconfig.get_path('scripts','nt_user'))"`) do set "SCRIPTS=%%i"
set "ZIG=%ZIGDIR%\zig.exe"
if not exist "%ZIG%" (
  echo [error] zig.exe not found under "%ZIGDIR%".
  exit /b 1
)

echo [4/6] Building gcc/g++/ar/ranlib shims in "%TOOLDIR%"...
if not exist "%TOOLDIR%" mkdir "%TOOLDIR%"
for %%n in (gcc g++ ar ranlib) do (
  "%ZIG%" cc -w -o "%TOOLDIR%\%%n.exe" "%SHIM%"
  if errorlevel 1 (
    echo [error] failed to build %%n.exe
    exit /b 1
  )
)

echo [5/6] Installing GNU make...
if exist "%TOOLDIR%\make.exe" (
  echo   GNU make already present: "%TOOLDIR%\make.exe"
) else (
  curl -k -fL --retry 3 -o "%TEMP%\philarmony-make.zip" "%MAKE_URL%"
  if errorlevel 1 (
    echo   [warn] could not download GNU make ^(network/TLS^). Install manually or run make from Git Bash.
  ) else (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "if(Test-Path (Join-Path $env:TEMP 'philarmony-make')){Remove-Item -Recurse -Force (Join-Path $env:TEMP 'philarmony-make')}; Expand-Archive -Path (Join-Path $env:TEMP 'philarmony-make.zip') -DestinationPath (Join-Path $env:TEMP 'philarmony-make') -Force; Copy-Item (Join-Path $env:TEMP 'philarmony-make\bin\make.exe') (Join-Path '%TOOLDIR%' 'make.exe') -Force"
    if errorlevel 1 (
      echo   [warn] could not extract GNU make from archive.
    ) else (
      echo   Installed GNU make to "%TOOLDIR%\make.exe"
    )
  )
)

echo [6/6] Updating user PATH and PHILARMONY_ZIG...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$dirs=@('%TOOLDIR%','%SCRIPTS%','%ZIGDIR%'); $p=[Environment]::GetEnvironmentVariable('Path','User'); if($null -eq $p){$p=''}; foreach($d in $dirs){ if($p -notlike ('*'+$d+'*')){ $p=($p.TrimEnd(';'))+';'+$d } }; [Environment]::SetEnvironmentVariable('Path',$p,'User'); [Environment]::SetEnvironmentVariable('PHILARMONY_ZIG','%ZIG%','User')"

echo.
echo Done. Open a NEW terminal ^(Git Bash recommended^), then run:
echo   make test
echo   make build ENV=esp32devkitc
echo PlatformIO CLI: %SCRIPTS%\pio.exe
exit /b 0
