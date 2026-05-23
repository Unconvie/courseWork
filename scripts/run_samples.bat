@echo off
REM Обработка всех ИСХОДНЫХ картинок в data\samples (без *-noiseFree, *-shapeOutline, *-copy).

setlocal
set "PROJECT_DIR=%~dp0.."
set "EXE=%PROJECT_DIR%\build\msvc-debug\bin\Debug\visionCore_app.exe"
set "SAMPLES=%PROJECT_DIR%\data\samples"

if not exist "%EXE%" (
    echo Сначала: scripts\build_msvc.bat
    exit /b 1
)

REM UTF-8 в консоли (русский текст в логах).
chcp 65001 >nul

REM GUI: запуск без аргументов. CLI: раскомментируй строку ниже.
"%EXE%"
REM "%EXE%" "%SAMPLES%" --save-noise-free --save-outline --save-report
echo.
echo Рядом с каждым исходником:
echo   *-noiseFree.*  *-shapeOutline.*  *-report.txt
endlocal
