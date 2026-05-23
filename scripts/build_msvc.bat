@echo off
REM Build visionCore (MSVC + vcpkg x64-windows).
REM Run from: x64 Native Tools Command Prompt for VS 2022
REM       or Developer PowerShell for VS 2022

setlocal
set "PROJECT_DIR=%~dp0.."
set "CMAKE=D:\progs\cmake\bin\cmake.exe"

if not exist "%CMAKE%" (
    echo ERROR: cmake not found at %CMAKE%
    exit /b 1
)

cd /d "%PROJECT_DIR%"

echo === Configure msvc-debug ===
"%CMAKE%" --preset msvc-debug
if errorlevel 1 exit /b 1

echo === Build Debug ===
"%CMAKE%" --build --preset msvc-debug
if errorlevel 1 exit /b 1

echo.
echo Build OK:
echo   %PROJECT_DIR%\build\msvc-debug\bin\Debug\visionCore_app.exe
echo.
echo Run GUI:  visionCore_app.exe
echo Run CLI:  visionCore_app.exe data\samples --save-outline
echo.

endlocal
