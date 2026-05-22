@echo off
REM Pipeline на тестовом изображении data\samples\тест1.png

setlocal
set "PROJECT_DIR=%~dp0.."
set "EXE=%PROJECT_DIR%\build\msvc-debug\bin\Debug\visionCore_app.exe"
set "TEST1=%PROJECT_DIR%\data\samples\тест1.png"

if not exist "%EXE%" (
    echo Сначала: scripts\build_msvc.bat
    exit /b 1
)

if not exist "%TEST1%" (
    echo Не найден файл: %TEST1%
    exit /b 1
)

"%EXE%" "%TEST1%" --save-noise-free --save-outline
echo.
echo Результаты рядом с исходником:
echo   тест1-noiseFree.png  тест1-shapeOutline.png
endlocal
