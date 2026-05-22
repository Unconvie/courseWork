# visionCore

Курсовая работа по теме:
**«Разработка модульной системы компьютерного зрения для обнаружения,
сегментации и классификации формы объектов на зашумлённых изображениях».**

**Фаза 3** — полный pipeline на OpenCV: фильтрация шума, детекция контуров,
извлечение признаков, классификация формы (круг, треугольник, прямоугольник,
квадрат, шестиугольник), разметка на изображении. Результаты пишутся в консоль
и при необходимости сохраняются рядом с исходником (`*-noiseFree`, `*-shapeOutline`).

---

## Структура проекта

```
courseWork/
├── CMakeLists.txt              описание сборки
├── CMakePresets.json           пресеты CMake (MinGW + vcpkg)
├── .gitignore                  что не пушим в git
├── README.md                   этот файл
├── include/visionCore/         публичные заголовки
│   ├── core/                       доменные типы (Image и др.)
│   ├── io/                         ввод-вывод (FileWalker, ImageIO)
│   └── infra/                      инфраструктура (Logger)
├── src/                        реализации
│   ├── core/
│   ├── io/
│   ├── infra/
│   └── main.cpp                точка входа CLI
└── data/samples/               тестовые изображения (не пушатся в git)
```

Логическое разделение:

- **core/** — доменные сущности, не зависящие ни от чего, кроме STL и OpenCV
  типов данных. Пример: `Image`.
- **io/** — ввод-вывод (диск, файловая система). Пример: `ImageIO`,
  `FileWalker`.
- **infra/** — общая инфраструктура (логи, в будущем — конфиг, метрики).
  Пример: `Logger`.
- **main.cpp** — тонкая «склейка»: парсит аргументы, дёргает компоненты,
  печатает результат. Должна оставаться тонкой даже когда появится pipeline.

---

## Требования к окружению (основной вариант — MSVC)

- **Visual Studio 2022** (Community подойдёт) с компонентом **«Разработка
  классических приложений на C++»**.
- **CMake** 3.21+.
- **vcpkg** в `D:\progs\vcpkg`.
- Пакеты для triplet **`x64-windows`**:
  ```powershell
  vcpkg install opencv4 nlohmann-json --triplet x64-windows
  ```
- `VCPKG_ROOT = D:\progs\vcpkg` (переменная пользователя).

Если путь к vcpkg другой — поправь `CMAKE_TOOLCHAIN_FILE` в
`CMakePresets.json`.

---

## Сборка MSVC (рекомендуется)

### Вариант A — скрипт

Из **x64 Native Tools Command Prompt for VS 2022** или **Developer PowerShell**:

```bat
cd /d "E:\uni\SEMESTER WORK\courseWork"
scripts\build_msvc.bat
```

### Вариант B — CMake вручную

```powershell
cd "E:\uni\SEMESTER WORK\courseWork"
cmake --preset msvc-debug
cmake --build --preset msvc-debug
```

Исполняемый файл:

`build\msvc-debug\bin\Debug\visionCore_app.exe`

### Вариант C — VS Code (сборка и запуск самой)

1. Открыть папку `courseWork` в VS Code.
2. Расширения: **CMake Tools**, **C/C++** (опционально для отладки).
3. Один раз из **Developer PowerShell for VS 2022**:
   ```powershell
   cd "E:\uni\SEMESTER WORK\courseWork"
   cmake --preset msvc-debug
   ```
4. В VS Code: **Terminal → Run Task → CMake: build (msvc-debug)**  
   или панель CMake Tools → **Build**.
5. Запуск с выводом в терминал: **Run Task → visionCore: run тест1**.
6. Отладка с точками останова: **Run and Debug → visionCore: тест1**.

Параметры алгоритма (пороги, Canny, minContourArea) — в файле
`include/visionCore/infra/AppConfig.hpp`. После правки — снова **Build**.
В Фазе 4 те же поля будут читаться из JSON (`--config`).

Файл `*-noiseFree` — это **рабочая копия для отчёта/отладки** (лёгкое
сглаживание bilateral), а не «идеально чистое» фото. Детекция идёт по
**оригиналу**, чтобы не терять тонкие обводки.

### Запуск

```powershell
cd "E:\uni\SEMESTER WORK\courseWork"
.\scripts\run_samples.bat
```

Или вручную:

```powershell
.\build\msvc-debug\bin\Debug\visionCore_app.exe --help
.\build\msvc-debug\bin\Debug\visionCore_app.exe data\samples --save-noise-free --save-outline
```

Флаги:

- `--save-noise-free` — очищенное изображение (`имя-noiseFree.jpg`)
- `--save-outline` — разметка контуров и подписей (`имя-shapeOutline.jpg`), по умолчанию включено
- `--no-outline` — не сохранять разметку

DLL OpenCV копируются в папку `bin\Debug` при сборке автоматически.

---

## Сборка MinGW (устаревший обходной путь)

Компилятор MSYS2 нужно запускать **из среды MSYS2**, а не из обычного
PowerShell. В пресете указан `D:/progs/MSYS2/usr/bin/g++.exe` — он
стабильно работает; `ucrt64/bin/g++.exe` в PowerShell может падать без
сообщения.

Открой **MSYS2 UCRT64** (или обычный MSYS2) из меню Пуск и выполни:

```bash
export PATH="/usr/bin:/ucrt64/bin:/d/progs/cmake/bin:$PATH"
cd "/e/uni/SEMESTER WORK/courseWork"
cmake --preset mingw-debug
cmake --build --preset mingw-debug
```

Исполняемый файл и DLL OpenCV окажутся в
`build/mingw-debug/bin/visionCore_app.exe`.

Для релизной сборки заменить `mingw-debug` на `mingw-release`.

### Проверка «сломан ли UCRT»

Команда `Test-Path "C:\Windows\System32\api-ms-win-crt-runtime-l1-1-0.dll"`
часто возвращает **False** — это **нормально** на Windows 10/11. Эти DLL
лежат в `WinSxS`, а не отдельным файлом в `System32`. Важнее:

- `Test-Path "C:\Windows\System32\ucrtbase.dll"` → должно быть **True**
- VC++ Redist установлен (в реестре `...\VC\Runtimes\x64`, `Installed = 1`)

---

## Сборка из VS Code

1. Открыть папку `courseWork` в VS Code.
2. Расширение **CMake Tools** → пресет **MinGW Debug**.
3. В `settings.json` workspace (см. `.vscode/settings.json`) задан
   `cmake.options` с toolchain vcpkg; **Configure** и **Build** лучше
   запускать из терминала **MSYS2** (см. выше), пока не настроен
   `terminal.integrated.profiles` под MSYS2.
4. Запуск: `build/mingw-debug/bin/visionCore_app.exe` + путь к картинке.

---

## Структура (после Фазы 3)

```
include/visionCore/
├── core/           Image
├── domain/         Detection, FeatureVector, Classification, Shape, shapes/*
├── pipeline/       интерфейсы + Pipeline + PipelineFactory
│   └── impl/       GaussianMedianFilter, ThresholdContourDetector, …
├── io/             FileWalker, ImageIO, PathUtils
└── infra/          Logger, AppConfig
```

## План дальнейших фаз

- **Фаза 2.** ✅ Интерфейсы pipeline, домен, класс `Pipeline`, эталоны форм.
- **Фаза 3.** ✅ Реализации pipeline, CLI, сохранение `noiseFree` / `shapeOutline`.
- **Фаза 4.** JSON-конфиг (`--config`), `--output-dir`, генератор тестовых изображений.
