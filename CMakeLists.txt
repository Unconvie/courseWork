# =============================================================================
# visionCore - модульная система компьютерного зрения для обнаружения
# и классификации формы объектов на зашумлённых изображениях.
#
# Сборка ведётся через CMake. Внешние зависимости (OpenCV, nlohmann_json)
# подтягиваются через vcpkg - конкретные пути и triplet задаются в
# CMakePresets.json, чтобы не зашивать их в этот файл.
# =============================================================================

# 3.21 нужен для CMakePresets v3 и для TARGET_RUNTIME_DLLS (используется ниже
# для автоматического копирования DLL зависимостей рядом с исполняемым файлом).
cmake_minimum_required(VERSION 3.21)

project(visionCore
    VERSION 0.1.0
    DESCRIPTION "Модульная система обнаружения и классификации формы объектов"
    LANGUAGES CXX)

# -----------------------------------------------------------------------------
# Глобальные настройки языка
# -----------------------------------------------------------------------------
# C++17 даёт нам std::filesystem, std::optional, std::string_view, structured
# bindings - всё, что нужно для каркаса. Расширения компилятора отключены,
# чтобы не привязываться к одному тулчейну.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Куда складывать результаты сборки
# -----------------------------------------------------------------------------
# Все исполняемые файлы и DLL кладём в один и тот же подкаталог bin/. Это
# нужно, чтобы при запуске .exe Windows мог найти зависимые DLL OpenCV
# в той же папке (см. add_custom_command в самом низу файла).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# -----------------------------------------------------------------------------
# Предупреждения компилятора
# -----------------------------------------------------------------------------
# Включаем максимально строгий набор. Если предупреждение появится, оно почти
# наверняка указывает на реальную проблему - не игнорируем.
if(MSVC)
    # /utf-8 — строки в исходниках и вывод в консоль/файлы как UTF-8 (русский текст).
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Внешние зависимости
# -----------------------------------------------------------------------------
# OpenCV - чтение/запись изображений, фильтры, контуры.
# nlohmann_json - чтение конфига (понадобится в Фазе 4).
# CMake находит их через toolchain-файл vcpkg, заданный в CMakePresets.json.
find_package(OpenCV REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# GLFW + Dear ImGui (GUI) — подтягиваются при конфигурации CMake (нужен интернет).
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
add_library(imgui_backend STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp
)
target_include_directories(imgui_backend PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui_backend PUBLIC glfw)

# -----------------------------------------------------------------------------
# Библиотека visionCore - вся бизнес-логика
# -----------------------------------------------------------------------------
# Делаем именно библиотеку (а не «всё в одном .exe»), чтобы позже:
#  - подключить её к утилитам (генератор тестов, бенчмарки),
#  - подключить к будущему GUI (заменится только main, ядро останется),
#  - писать модульные тесты без перекомпиляции всего проекта.
add_library(visionCore STATIC
    src/core/Image.cpp
    src/domain/ShapeKind.cpp
    src/domain/Classification.cpp
    src/domain/shapes/Circle.cpp
    src/domain/shapes/Triangle.cpp
    src/domain/shapes/Rectangle.cpp
    src/domain/shapes/Square.cpp
    src/domain/shapes/Hexagon.cpp
    src/io/FileWalker.cpp
    src/io/ImageIO.cpp
    src/infra/Logger.cpp
    src/io/PathUtils.cpp
    src/pipeline/Pipeline.cpp
    src/pipeline/PipelineFactory.cpp
    src/pipeline/impl/GaussianMedianFilter.cpp
    src/pipeline/impl/ThresholdContourDetector.cpp
    src/pipeline/impl/ContourFeatureExtractor.cpp
    src/pipeline/impl/ShapeClassifier.cpp
    src/pipeline/impl/OpenCvAnnotator.cpp
    src/app/BatchRunner.cpp
)

# Публичные заголовки видны и самой библиотеке, и её потребителям (main и т.д.).
target_include_directories(visionCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# OpenCV и json - часть публичного интерфейса (Image держит cv::Mat и т.д.),
# поэтому линкуем PUBLIC: потребители visionCore тоже получают эти зависимости.
target_link_libraries(visionCore PUBLIC
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)

# -----------------------------------------------------------------------------
# Исполняемый файл - точка входа CLI
# -----------------------------------------------------------------------------
add_executable(visionCore_app
    src/main.cpp
    src/gui/GuiApp.cpp
    src/gui/NativeDialogsWin32.cpp
)
target_link_libraries(visionCore_app PRIVATE
    visionCore
    imgui_backend
)
if(WIN32)
    target_link_libraries(visionCore_app PRIVATE
        opengl32
        gdi32
        shell32
        ole32
    )
endif()

# Обходной вариант только для сломанного MinGW-пресета (см. mingw-debug).
if(MINGW)
    target_link_options(visionCore_app PRIVATE
        -static-libstdc++
        -static-libgcc
    )
endif()

# -----------------------------------------------------------------------------
# Авто-копирование DLL рядом с .exe (Windows)
# -----------------------------------------------------------------------------
# Для MSVC (x64-windows) vcpkg копирует DLL через applocal.ps1, если в пресете
# включён VCPKG_APPLOCAL_DEPS=ON. Дублируем копированием из installed/.../bin
# на случай, если PowerShell недоступен из PATH при сборке.
if(WIN32)
    add_custom_command(TARGET visionCore_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<IF:$<CONFIG:Debug>,${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin,${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin>"
            "$<TARGET_FILE_DIR:visionCore_app>"
        VERBATIM
    )
endif()
