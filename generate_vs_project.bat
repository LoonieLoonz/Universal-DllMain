@echo off
REM ============================================================================
REM Generate Visual Studio Solution
REM ============================================================================

setlocal

set VS_VERSION=Visual Studio 17 2022
set SOLUTION_DIR=vs_solution

echo Generating Visual Studio Solution...
echo.

REM Create solution directory
if not exist "%SOLUTION_DIR%" mkdir "%SOLUTION_DIR%"

REM Generate with CRT
echo [1/2] Generating configuration WITH CRT...
cmake -S . -B "%SOLUTION_DIR%/with_crt" ^
    -G "%VS_VERSION%" ^
    -DUSE_CRT=ON ^
    -DBUILD_SHARED=ON

if %ERRORLEVEL% NEQ 0 (
    echo Error: Failed to generate WITH CRT configuration
    exit /b 1
)

REM Generate without CRT
echo [2/2] Generating configuration WITHOUT CRT...
cmake -S . -B "%SOLUTION_DIR%/without_crt" ^
    -G "%VS_VERSION%" ^
    -DUSE_CRT=OFF ^
    -DBUILD_SHARED=ON

if %ERRORLEVEL% NEQ 0 (
    echo Error: Failed to generate WITHOUT CRT configuration
    exit /b 1
)

echo.
echo ============================================================================
echo Visual Studio solutions generated successfully!
echo.
echo With CRT:    %SOLUTION_DIR%\with_crt\UniversalDLL.sln
echo Without CRT: %SOLUTION_DIR%\without_crt\UniversalDLL.sln
echo.
echo Open these files in Visual Studio to build the project.
echo ============================================================================

endlocal