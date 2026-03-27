@echo off
setlocal
chcp 65001 >nul 2>&1

set BUILD_DIR=build
set BUILD_TYPE=Release

if "%~1"=="debug"   set BUILD_TYPE=Debug
if "%~1"=="release" set BUILD_TYPE=Release
if "%~1"=="clean" (
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
    echo Cleaned.
    exit /b 0
)

echo ================================================
echo   SimpleMarkdown Build [%BUILD_TYPE%]
echo ================================================

REM ---- Find Visual Studio ----
set VCVARS=
for %%D in (C D) do (
    for %%V in (2022 2019) do (
        for %%E in (Community Professional Enterprise) do (
            if not defined VCVARS (
                if exist "%%D:\Program Files (x86)\Microsoft Visual Studio\%%V\%%E\VC\Auxiliary\Build\vcvarsall.bat" (
                    set "VCVARS=%%D:\Program Files (x86)\Microsoft Visual Studio\%%V\%%E\VC\Auxiliary\Build\vcvarsall.bat"
                )
                if exist "%%D:\Program Files\Microsoft Visual Studio\%%V\%%E\VC\Auxiliary\Build\vcvarsall.bat" (
                    set "VCVARS=%%D:\Program Files\Microsoft Visual Studio\%%V\%%E\VC\Auxiliary\Build\vcvarsall.bat"
                )
            )
        )
    )
)
if not defined VCVARS (
    echo [ERROR] Visual Studio not found.
    exit /b 1
)

echo [1/3] Setting up MSVC environment...
call "%VCVARS%" x64 >nul 2>&1

REM ---- Find Qt5 (prefer 5.12.9) ----
set QT_DIR=
if defined Qt5_DIR set "QT_DIR=%Qt5_DIR%"
if defined QT_DIR goto qt_found
for %%D in (C D E) do (
    if not defined QT_DIR if exist "%%D:\Qt\Qt5.12.9\5.12.9\msvc2017_64\bin\Qt5Core.dll" set "QT_DIR=%%D:\Qt\Qt5.12.9\5.12.9\msvc2017_64"
)
if defined QT_DIR goto qt_found
for %%D in (C D E) do (
    for /d %%Q in ("%%D:\Qt\Qt5*") do (
        if not defined QT_DIR for /d %%V in ("%%Q\5*") do (
            if not defined QT_DIR for /d %%A in ("%%V\msvc*_64") do set "QT_DIR=%%A"
        )
    )
    for /d %%V in ("%%D:\Qt\5*") do (
        if not defined QT_DIR for /d %%A in ("%%V\msvc*_64") do set "QT_DIR=%%A"
    )
)
:qt_found
if defined QT_DIR (
    set "QT_CMAKE_OPT=-DCMAKE_PREFIX_PATH=%QT_DIR%"
    echo       Qt5: %QT_DIR%
) else (
    set QT_CMAKE_OPT=
)

REM ---- Configure ----
if not exist %BUILD_DIR%\CMakeCache.txt (
    echo [2/3] CMake configure...
    cmake -S . -B %BUILD_DIR% -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %QT_CMAKE_OPT%
    if errorlevel 1 (
        echo [ERROR] CMake configure failed.
        echo         Make sure Qt5 is installed. Set Qt5_DIR or CMAKE_PREFIX_PATH if needed.
        exit /b 1
    )
) else (
    echo [2/3] CMake cache exists, skipping configure.
)

REM ---- Build ----
echo [3/3] Building...
cmake --build %BUILD_DIR%
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo ================================================
echo   Build succeeded: %BUILD_DIR%\app\SimpleMarkdown.exe
echo ================================================
