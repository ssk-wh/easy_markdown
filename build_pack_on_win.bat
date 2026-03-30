@echo off
setlocal enabledelayedexpansion

REM ===============================================
REM SimpleMarkdown - Build, Collect & Pack Script
REM ===============================================

cd /d "%~dp0"

echo.
echo ================================================
echo   Step 1/3: Building SimpleMarkdown...
echo ================================================
call build_on_win.bat
if errorlevel 1 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo.
echo ================================================
echo   Step 2/3: Collecting dependencies...
echo ================================================
chcp.com 65001 > nul 2>&1
python installer\collect_dist.py build\app
if errorlevel 1 (
    echo [ERROR] Dependency collection failed!
    exit /b 1
)

echo.
echo ================================================
echo   Step 3/3: Packing NSIS installer...
echo ================================================

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
"!NSIS_EXE!" SimpleMarkdown.nsi
if errorlevel 1 (
    echo [ERROR] NSIS packing failed!
    cd ..
    exit /b 1
)
cd ..

echo.
echo ================================================
echo   SUCCESS: All steps completed!
echo ================================================
echo.
echo Installer location:
dir installer\SimpleMarkdown-*.exe
echo.
pause
