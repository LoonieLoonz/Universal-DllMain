@echo off
REM ============================================================================
REM Build script for Universal DLL - Windows
REM Supports multiple configurations with/without CRT
REM ============================================================================

setlocal enabledelayedexpansion

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake not found in PATH
    exit /b 1
)

REM Parse arguments
set BUILD_TYPE=Release
set USE_CRT=ON
set BUILD_DIR=build
set GENERATOR=Visual Studio 17 2022

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--debug" set BUILD_TYPE=Debug
if /i "%~1"=="--release" set BUILD_TYPE=Release
if /i "%~1"=="--no-crt" set USE_CRT=OFF
if /i "%~1"=="--crt" set USE_CRT=ON
if /i "%~1"=="--generator" (
    set GENERATOR=%~2
    shift
)
if /i "%~1"=="--build-dir" (
    set BUILD_DIR=%~2
    shift
)
if /i "%~1"=="--help" goto show_help
shift
goto parse_args
:end_parse

echo ============================================================================
echo Universal DLL Build Configuration
echo ============================================================================
echo Build Type: %BUILD_TYPE%
echo Use CRT: %USE_CRT%
echo Build Directory: %BUILD_DIR%
echo Generator: %GENERATOR%
echo ============================================================================
echo.

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure with CMake
echo Configuring project...
cmake -S . -B "%BUILD_DIR%" ^
    -G "%GENERATOR%" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DUSE_CRT=%USE_CRT% ^
    -DBUILD_SHARED=ON

if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake configuration failed
    exit /b 1
)

REM Build
echo.
echo Building project...
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%

if %ERRORLEVEL% NEQ 0 (
    echo Error: Build failed
    exit /b 1
)

echo.
echo ============================================================================
echo Build completed successfully!
echo Output: %BUILD_DIR%\%BUILD_TYPE%\UniversalDLL.dll
echo ============================================================================

goto end

:show_help
echo Usage: build.bat [options]
echo.
echo Options:
echo   --debug          Build in Debug mode (default: Release)
echo   --release        Build in Release mode
echo   --no-crt         Build without C Runtime Library
echo   --crt            Build with C Runtime Library (default)
echo   --generator GEN  Specify CMake generator (default: Visual Studio 17 2022)
echo   --build-dir DIR  Specify build directory (default: build)
echo   --help           Show this help message
echo.
echo Examples:
echo   build.bat --release --crt
echo   build.bat --debug --no-crt
echo   build.bat --release --no-crt --build-dir build-nocrt

:end
endlocal