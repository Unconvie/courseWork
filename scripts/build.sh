#!/usr/bin/env bash
# Сборка visionCore из среды MSYS2 (bash).
# Запускать из «MSYS2 UCRT64» или «MSYS2 MSYS»:
#   cd "/e/uni/SEMESTER WORK/courseWork"
#   bash scripts/build.sh
#
# Не запускайте cmake из обычного PowerShell без MSYS2 PATH —
# компилятор ucrt64 может не стартовать (ошибка 0xC0000139).

set -euo pipefail

export PATH="/usr/bin:/ucrt64/bin:/d/progs/cmake/bin:${PATH:-}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_DIR}"

echo "=== Configure (mingw-debug) ==="
cmake --preset mingw-debug

echo "=== Build ==="
cmake --build --preset mingw-debug

echo ""
echo "Готово: ${PROJECT_DIR}/build/mingw-debug/bin/visionCore_app.exe"
echo "Запуск (из cmd/PowerShell, из папки bin):"
echo "  visionCore_app.exe --help"
echo "  visionCore_app.exe путь\\к\\картинке_или_папке"
