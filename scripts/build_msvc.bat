@echo off
REM Сборка visionCore под MSVC + vcpkg triplet x64-windows.
REM Запускать из "x64 Native Tools Command Prompt for VS 2022"
REM или "Developer PowerShell for VS 2022" (из меню Пуск).

setlocal
set "PROJECT_DIR=%~dp0.."
set "CMAKE=D:\progs\cmake\bin\cmake.exe"

if not exist "%CMAKE%" (
    echo Ошибка: cmake не найден по пути %CMAKE%
    exit /b 1
)

cd /d "%PROJECT_DIR%"

echo === Configure (msvc-debug) ===
"%CMAKE%" --preset msvc-debug
if errorlevel 1 exit /b 1

echo === Build Debug ===
"%CMAKE%" --build --preset msvc-debug
if errorlevel 1 exit /b 1

echo.
echo Сборка готова:
echo   %PROJECT_DIR%\build\msvc-debug\bin\Debug\visionCore_app.exe
echo.
echo ВАЖНО: build_msvc.bat только КОМПИЛИРУЕТ. Копии -copy НЕ создаются при сборке.
echo.
echo Чтобы создать копии, ЗАПУСТИ программу, например:
echo   scripts\run_samples.bat
echo или:
echo   build\msvc-debug\bin\Debug\visionCore_app.exe data\samples
echo.
echo Файлы *-copy.jpg появятся в той же папке, где лежит оригинал (data\samples).

endlocal
