@echo off
REM Generate test images (shapes + noise) into data\samples

setlocal
set "PROJECT_DIR=%~dp0.."
set "EXE=%PROJECT_DIR%\build\msvc-debug\bin\Debug\visionCore_app.exe"
set "SAMPLES=%PROJECT_DIR%\data\samples"

if not exist "%EXE%" (
    echo Build first: scripts\build_msvc.bat
    exit /b 1
)

chcp 65001 >nul
"%EXE%" --generate-samples "%SAMPLES%"
endlocal
