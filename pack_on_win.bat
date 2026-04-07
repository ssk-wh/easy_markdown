@echo off
setlocal enabledelayedexpansion

REM ===============================================
REM SimpleMarkdown - Windows Pack Script
REM Usage: run build_on_win.bat first, then this script
REM ===============================================

cd /d "%~dp0"

REM Check build output exists
if not exist "build\src\app\SimpleMarkdown.exe" (
    echo [ERROR] build\src\app\SimpleMarkdown.exe not found.
    echo         Please run build_on_win.bat first.
    exit /b 1
)

echo.
echo ================================================
echo   Step 1/2: Collecting dependencies...
echo ================================================
chcp.com 65001 > nul 2>&1
python installer\collect_dist.py build\src\app
if errorlevel 1 (
    echo [ERROR] Dependency collection failed!
    exit /b 1
)

echo.
echo ================================================
echo   Step 2/2: Packing NSIS installer...
echo ================================================

REM Extract version from CHANGELOG.md (first ## [x.y.z] line)
set "APP_VER="
for /f "tokens=2 delims=[]" %%V in ('findstr /r "\[[0-9]*\.[0-9]*\.[0-9]*\]" CHANGELOG.md') do (
    if not defined APP_VER set "APP_VER=%%V"
)
if not defined APP_VER (
    echo [ERROR] Cannot extract version from CHANGELOG.md
    exit /b 1
)
echo   Version: %APP_VER% ^(from CHANGELOG.md^)

REM Determine NSIS path
set "NSIS_EXE="
if exist "C:\Program Files (x86)\NSIS\makensis.exe" (
    set "NSIS_EXE=C:\Program Files (x86)\NSIS\makensis.exe"
) else if exist "C:\Program Files\NSIS\makensis.exe" (
    set "NSIS_EXE=C:\Program Files\NSIS\makensis.exe"
)

if not defined NSIS_EXE (
    echo [ERROR] NSIS not found. Please install NSIS first.
    exit /b 1
)

echo   NSIS path: !NSIS_EXE!
echo.

cd installer
"!NSIS_EXE!" /DAPP_VERSION=%APP_VER% SimpleMarkdown.nsi
if errorlevel 1 (
    echo [ERROR] NSIS packing failed!
    cd ..
    exit /b 1
)
cd ..

echo.
echo ================================================
echo   SUCCESS: Pack completed!
echo ================================================
echo.
echo Installer location:
dir installer\SimpleMarkdown-*.exe
echo.
