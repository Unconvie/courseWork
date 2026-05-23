@echo off
REM Run visionCore: GUI (no args) or CLI on data\samples folder.

setlocal
set "PROJECT_DIR=%~dp0.."
set "EXE=%PROJECT_DIR%\build\msvc-debug\bin\Debug\visionCore_app.exe"
set "SAMPLES=%PROJECT_DIR%\data\samples"

if not exist "%EXE%" (
    echo Build first: scripts\build_msvc.bat
    exit /b 1
)

chcp 65001 >nul

REM GUI (default):
"%EXE%"

REM CLI on folder (uncomment):
REM "%EXE%" "%SAMPLES%" --save-noise-free --save-outline --save-report

endlocal
