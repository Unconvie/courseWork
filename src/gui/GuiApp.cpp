#include "visionCore/gui/GuiApp.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>

#include "visionCore/app/BatchRunner.hpp"
#include "visionCore/app/OutputOptions.hpp"
#include "visionCore/gui/NativeDialogs.hpp"
#include "visionCore/infra/AppConfig.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace visionCore::gui {

namespace {

enum class RunState { Idle, Done };

struct GuiState {
    char inputPath[1024] = "data/samples";
    infra::AppConfig config;
    app::OutputOptions outputs;
    RunState runState = RunState::Idle;
    std::string statusLine = u8"Готов к работе.";
    std::vector<std::string> lastLog;
    bool showLog = false;
};

void setupImGuiFont() {
    ImGuiIO& io = ImGui::GetIO();
#if defined(_WIN32)
    const char* fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
    FILE* f = nullptr;
    if (fopen_s(&f, fontPath, "rb") == 0 && f) {
        std::fclose(f);
        io.Fonts->AddFontFromFileTTF(
            fontPath, 18.0f, nullptr,
            io.Fonts->GetGlyphRangesCyrillic());
        return;
    }
#endif
    io.Fonts->AddFontDefault();
}

/** Подсказка к последнему виджету (наведи мышь). */
void itemTip(const char* tip) {
    if (tip == nullptr || tip[0] == '\0') {
        return;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(420.0f);
        ImGui::TextUnformatted(tip);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void drawConfigBool(const char* label, bool& value, const char* tip) {
    ImGui::Checkbox(label, &value);
    itemTip(tip);
}

void drawConfigInt(const char* label, int& value, int minV, int maxV,
                   const char* tip) {
    ImGui::SliderInt(label, &value, minV, maxV);
    itemTip(tip);
}

void drawConfigFloat(const char* label, float& value, float minV, float maxV,
                     const char* tip) {
    ImGui::SliderFloat(label, &value, minV, maxV);
    itemTip(tip);
}

void drawNoiseFreeConfig(infra::AppConfig& cfg) {
    const bool open = ImGui::CollapsingHeader(
        u8"Фильтр шума (noiseFree)", ImGuiTreeNodeFlags_DefaultOpen);
    itemTip(u8"Параметры сглаживания для файла *-noiseFree. На поиск контуров "
            u8"не влияют — детекция идёт по оригиналу.");
    if (!open) {
        return;
    }

    drawConfigInt(
        "bilateralDiameter", cfg.bilateralDiameter, 0, 15,
        u8"Диаметр bilateral-фильтра на цветном изображении. 0 — не применять. "
        u8"Больше значение — сильнее сглаживание с сохранением краёв.");

    drawConfigFloat(
        "gaussianSigma", reinterpret_cast<float&>(cfg.gaussianSigma), 0.f, 5.f,
        u8"Дополнительное размытие по яркости (grayscale) для noiseFree. "
        u8"0 — без Gaussian. Увеличивает «мягкость» картинки.");

    drawConfigInt(
        "medianKernel", cfg.medianKernel, 0, 15,
        u8"Медианный фильтр (только нечётные ≥3). 0 — выкл. Хорош против "
        u8"соль-перец шума, но размывает мелкие детали.");

    drawConfigInt(
        "jpegQuality", cfg.jpegQuality, 50, 100,
        u8"Качество JPEG при сохранении noiseFree и shapeOutline (50–100). "
        u8"PNG сохраняется без потерь независимо от этого параметра.");
}

void drawDetectionConfig(infra::AppConfig& cfg) {
    const bool open = ImGui::CollapsingHeader(
        u8"Детекция (контуры)", ImGuiTreeNodeFlags_DefaultOpen);
    itemTip(u8"Как искать объекты на изображении: пороги, Canny, круги Hough. "
            u8"Эти настройки сильнее всего влияют на число и качество рамок.");
    if (!open) {
        return;
    }

    drawConfigFloat(
        "detectGaussianSigma",
        reinterpret_cast<float&>(cfg.detectGaussianSigma), 0.f, 5.f,
        u8"Сглаживание перед детекцией. 0 — максимально резкие контуры "
        u8"(лучше для тонких обводок). Больше 1 — сглаживает и может склеить "
        u8"близкие линии.");

    drawConfigBool(
        "adaptiveThreshold", cfg.adaptiveThreshold,
        u8"Адаптивный порог по локальным участкам изображения. Обычно лучше "
        u8"на неравномерном освещении. Выкл. — фиксированный thresholdValue.");

    if (!cfg.adaptiveThreshold) {
        drawConfigInt(
            "thresholdValue", cfg.thresholdValue, 0, 255,
            u8"Порог яркости 0–255 для бинаризации (если adaptiveThreshold выкл.). "
            u8"Меньше — больше пикселей считается объектом.");
    }

    drawConfigBool(
        "useClahe", cfg.useClahe,
        u8"CLAHE — усиление локального контраста. Помогает видеть бледные "
        u8"линии на белом фоне (слабый квадрат и т.п.).");

    if (cfg.useClahe) {
        drawConfigFloat(
            "claheClipLimit", reinterpret_cast<float&>(cfg.claheClipLimit), 1.f,
            8.f,
            u8"Сила CLAHE. Выше — контрастнее, но могут появиться артефакты. "
            u8"Попробуй 3–4 для бледных фигур.");
    }

    drawConfigBool(
        "useCannyEdges", cfg.useCannyEdges,
        u8"Дополнительный поиск границ оператором Canny. Важен для тонких "
        u8"контуров: треугольник, многоугольник, квадрат-обводка.");

    if (cfg.useCannyEdges) {
        drawConfigInt(
            "cannyLow", cfg.cannyLow, 1, 255,
            u8"Нижний порог Canny (основной проход). Меньше — больше слабых "
            u8"границ, больше шума.");

        drawConfigInt(
            "cannyHigh", cfg.cannyHigh, 1, 255,
            u8"Верхний порог Canny. Должен быть больше cannyLow. Сильные "
            u8"контуры должны попадать выше этого порога.");

        drawConfigInt(
            "faintCannyLow", cfg.faintCannyLow, 1, 255,
            u8"Нижний порог второго (слабого) прохода Canny для едва видимых "
            u8"линий.");

        drawConfigInt(
            "faintCannyHigh", cfg.faintCannyHigh, 1, 255,
            u8"Верхний порог слабого Canny. Используется вместе с faintCannyLow.");

        drawConfigFloat(
            "minFaintContourArea",
            reinterpret_cast<float&>(cfg.minFaintContourArea), 20.f, 2000.f,
            u8"Мин. площадь контура (px²) для слабого Canny. Меньше — больше "
            u8"мелких срабатываний от слабых линий.");

        drawConfigFloat(
            "minCannyContourArea",
            reinterpret_cast<float&>(cfg.minCannyContourArea), 50.f, 3000.f,
            u8"Мин. площадь контура (px²) для основного Canny. Отсекает "
            u8"короткие обрывки линий.");
    }

    drawConfigBool(
        "useHoughCircles", cfg.useHoughCircles,
        u8"Поиск окружностей методом Хафа. Помогает отдельно находить залитые "
        u8"и пересекающиеся круги, которые сливаются в один контур.");

    if (cfg.useHoughCircles) {
        drawConfigFloat(
            "houghDp", reinterpret_cast<float&>(cfg.houghDp), 1.f, 3.f,
            u8"Обратное разрешение аккумулятора Hough (dp). 1 — точнее и "
            u8"медленнее, 2 — быстрее.");

        drawConfigFloat(
            "houghMinDist", reinterpret_cast<float&>(cfg.houghMinDist), 10.f,
            200.f,
            u8"Мин. расстояние между центрами найденных кругов (px). Меньше — "
            u8"больше кругов рядом (для пары пересекающихся).");

        drawConfigFloat(
            "houghParam1", reinterpret_cast<float&>(cfg.houghParam1), 10.f,
            200.f,
            u8"Верхний порог Canny внутри Hough (param1). Чем выше — меньше "
            u8"чувствительность к слабым границам.");

        drawConfigFloat(
            "houghParam2", reinterpret_cast<float&>(cfg.houghParam2), 5.f, 80.f,
            u8"Порог «голосования» (param2). Выше — меньше ложных кругов, "
            u8"но можно пропустить слабые. Ниже — больше кругов.");

        drawConfigInt(
            "houghMinRadius", cfg.houghMinRadius, 5, 300,
            u8"Минимальный радиус искомого круга в пикселях.");

        drawConfigInt(
            "houghMaxRadius", cfg.houghMaxRadius, 10, 500,
            u8"Максимальный радиус искомого круга в пикселях.");
    }

    drawConfigFloat(
        "minContourArea", reinterpret_cast<float&>(cfg.minContourArea), 10.f,
        2000.f,
        u8"Мин. площадь залитого контура (px²). Меньше — больше мелких "
        u8"объектов и шума. Больше — только крупные фигуры.");

    drawConfigFloat(
        "nestedOutlineKeepMinRatio",
        reinterpret_cast<float&>(cfg.nestedOutlineKeepMinRatio), 0.01f, 0.5f,
        u8"Доля площади вложенного контура от внешнего: если обводка достаточно "
        u8"большая — сохраняем (квадрат внутри круга).");

    drawConfigFloat(
        "nestedBlobSuppressMaxRatio",
        reinterpret_cast<float&>(cfg.nestedBlobSuppressMaxRatio), 0.05f, 0.6f,
        u8"Удалять мелкие «пятна» внутри большого круга (ложные квадраты внутри "
        u8"залитого круга), если площадь меньше этой доли.");

    drawConfigInt(
        "minBboxSide", cfg.minBboxSide, 1, 50,
        u8"Мин. сторона ограничивающего прямоугольника (px). Отсекает "
        u8"линии толщиной 1–2 пикселя.");

    drawConfigFloat(
        "maxContourAreaRatio",
        reinterpret_cast<float&>(cfg.maxContourAreaRatio), 0.3f, 0.99f,
        u8"Макс. доля площади кадра для контура. Отсекает «контур всего "
        u8"изображения» и рамку по краю.");

    drawConfigFloat(
        "contourDedupeIou", reinterpret_cast<float&>(cfg.contourDedupeIou), 0.1f,
        0.95f,
        u8"Порог IoU bbox для слияния дубликатов. Выше — агрессивнее объединяет "
        u8"перекрывающиеся детекции, оставляя одну.");
}

void drawClassificationConfig(infra::AppConfig& cfg) {
    const bool open = ImGui::CollapsingHeader(
        u8"Классификация форм", ImGuiTreeNodeFlags_DefaultOpen);
    itemTip(u8"Как по контуру выбрать форму: круг, треугольник, квадрат и т.д. "
            u8"Это правила по числу вершин и круглости, не нейросеть.");
    if (!open) {
        return;
    }

    drawConfigFloat(
        "minClassificationConfidence",
        reinterpret_cast<float&>(cfg.minClassificationConfidence), 0.05f,
        0.95f,
        u8"Мин. уверенность лучшего класса (0–1). Ниже порога подпись "
        u8"Unknown. Выше — чаще Unknown; ниже — больше ошибочных меток.");

    drawConfigFloat(
        "contourApproxEpsilon",
        reinterpret_cast<float&>(cfg.contourApproxEpsilon), 0.005f, 0.08f,
        u8"Точность упрощения контура (Дуглас–Пёкер), доля периметра. Меньше — "
        u8"больше вершин (лучше для многоугольника). Больше — грубее, ближе к "
        u8"кругу.");
}

void drawUi(GuiState& state) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("visionCore", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                     | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu(u8"Справка")) {
            if (ImGui::MenuItem(u8"О программе")) {
                state.showLog = false;
                state.statusLine =
                    u8"visionCore — курсовой проект. CLI: visionCore_app.exe "
                    u8"<путь> [опции]. Наведи мышь на параметр — подсказка.";
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::TextUnformatted(u8"Входные данные");
    itemTip(u8"Путь к одному изображению или к папке. В папке обрабатываются "
            u8"только исходники (без файлов -noiseFree, -shapeOutline, -copy).");

    ImGui::InputText("##path", state.inputPath, sizeof(state.inputPath));
    itemTip(u8"Можно ввести путь вручную или выбрать кнопками справа.");

    ImGui::SameLine();
    if (ImGui::Button(u8"Файл")) {
        std::string picked;
        if (pickImageFile(picked)) {
            std::snprintf(state.inputPath, sizeof(state.inputPath), "%s",
                          picked.c_str());
        }
    }
    itemTip(u8"Выбрать один файл: PNG, JPG, BMP, TIFF.");

    ImGui::SameLine();
    if (ImGui::Button(u8"Папка")) {
        std::string picked;
        if (pickFolder(picked)) {
            std::snprintf(state.inputPath, sizeof(state.inputPath), "%s",
                          picked.c_str());
        }
    }
    itemTip(u8"Выбрать папку — обработаются все подходящие изображения в ней.");

    ImGui::Separator();
    ImGui::TextUnformatted(u8"Создавать файлы");
    itemTip(u8"Какие результаты сохранить рядом с каждым исходным файлом.");

    ImGui::Checkbox(u8"*-shapeOutline", &state.outputs.saveOutline);
    itemTip(u8"Копия исходника с рамками, контурами и подписью формы "
            u8"(Circle 40% и т.д.). Имя: имя-shapeOutline.png");

    ImGui::SameLine();
    ImGui::Checkbox(u8"*-noiseFree", &state.outputs.saveNoiseFree);
    itemTip(u8"Сглаженная копия для отчёта (bilateral и др.). Имя: имя-noiseFree.png. "
            u8"На детекцию не влияет.");

    ImGui::SameLine();
    ImGui::Checkbox(u8"*-report.txt", &state.outputs.saveReport);
    itemTip(u8"Текстовый отчёт: число объектов, bbox, центроид, уверенность, "
            u8"распределение по формам. Имя: имя-report.txt");

    ImGui::Separator();
    ImGui::TextDisabled(u8"(?) Наведи курсор на параметр или кнопку — подсказка");

    ImGui::BeginChild("config", ImVec2(0, -80), true);
    drawNoiseFreeConfig(state.config);
    drawDetectionConfig(state.config);
    drawClassificationConfig(state.config);
    ImGui::EndChild();

    if (ImGui::Button(u8"Запустить обработку", ImVec2(200, 36))) {
        app::BatchRequest req;
        req.inputPath = state.inputPath;
        req.config = state.config;
        req.outputs = state.outputs;

        const app::BatchResult batch = app::runBatch(req);
        state.lastLog = batch.logLines;
        state.runState = RunState::Done;
        state.statusLine = batch.summary;
        state.showLog = true;
    }
    itemTip(u8"Запустить pipeline для выбранного файла или всех исходников "
            u8"в папке. Может занять время на большой папке.");

    ImGui::SameLine();
    if (ImGui::Button(u8"Журнал")) {
        state.showLog = !state.showLog;
    }
    itemTip(u8"Показать подробный лог последнего запуска (то же, что в консоли "
            u8"в CLI-режиме).");

    ImGui::Separator();
    if (state.runState == RunState::Done) {
        ImGui::TextColored(ImVec4(0.1f, 0.7f, 0.2f, 1.f), u8"Работа завершена.");
        itemTip(u8"Обработка последнего запуска закончена. Результаты на диске "
                u8"(если включены галочки выше).");
    }
    ImGui::TextWrapped("%s", state.statusLine.c_str());

    if (state.showLog && !state.lastLog.empty()) {
        ImGui::Begin(u8"Журнал обработки", &state.showLog);
        itemTip(u8"Построчный лог: файлы, найденные объекты, пути сохранения.");
        ImGui::BeginChild("logscroll");
        for (const auto& line : state.lastLog) {
            ImGui::TextWrapped("%s", line.c_str());
        }
        ImGui::EndChild();
        ImGui::End();
    }

    ImGui::End();
}

}  // anonymous namespace

int runGuiApp() {
#if defined(_WIN32)
    FreeConsole();
#endif

    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    GLFWwindow* window =
        glfwCreateWindow(1100, 720, "visionCore", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    setupImGuiFont();

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    GuiState state;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawUi(state);

        ImGui::Render();
        int w = 0;
        int h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.12f, 0.12f, 0.14f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

}  // namespace visionCore::gui
