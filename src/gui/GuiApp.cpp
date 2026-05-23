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
#if defined(_WIN32)
    if (fopen_s(&f, fontPath, "rb") == 0 && f)
#else
    if ((f = std::fopen(fontPath, "rb")))
#endif
    {
        std::fclose(f);
        io.Fonts->AddFontFromFileTTF(
            fontPath, 18.0f, nullptr,
            io.Fonts->GetGlyphRangesCyrillic());
        return;
    }
#endif
    io.Fonts->AddFontDefault();
}

void drawConfigBool(const char* label, bool& value) {
    ImGui::Checkbox(label, &value);
}

void drawConfigInt(const char* label, int& value, int minV, int maxV) {
    ImGui::SliderInt(label, &value, minV, maxV);
}

void drawConfigFloat(const char* label, float& value, float minV, float maxV) {
    ImGui::SliderFloat(label, &value, minV, maxV);
}

void drawDetectionConfig(infra::AppConfig& cfg) {
    if (!ImGui::CollapsingHeader("Детекция (контуры)", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    drawConfigFloat("detectGaussianSigma", reinterpret_cast<float&>(cfg.detectGaussianSigma),
                    0.f, 5.f);
    drawConfigBool("adaptiveThreshold", cfg.adaptiveThreshold);
    if (!cfg.adaptiveThreshold) {
        drawConfigInt("thresholdValue", cfg.thresholdValue, 0, 255);
    }
    drawConfigBool("useClahe", cfg.useClahe);
    if (cfg.useClahe) {
        drawConfigFloat("claheClipLimit", reinterpret_cast<float&>(cfg.claheClipLimit),
                        1.f, 8.f);
    }
    drawConfigBool("useCannyEdges", cfg.useCannyEdges);
    if (cfg.useCannyEdges) {
        drawConfigInt("cannyLow", cfg.cannyLow, 1, 255);
        drawConfigInt("cannyHigh", cfg.cannyHigh, 1, 255);
        drawConfigInt("faintCannyLow", cfg.faintCannyLow, 1, 255);
        drawConfigInt("faintCannyHigh", cfg.faintCannyHigh, 1, 255);
        drawConfigFloat("minFaintContourArea",
                        reinterpret_cast<float&>(cfg.minFaintContourArea), 20.f, 2000.f);
        drawConfigFloat("minCannyContourArea",
                        reinterpret_cast<float&>(cfg.minCannyContourArea), 50.f, 3000.f);
    }
    drawConfigBool("useHoughCircles", cfg.useHoughCircles);
    if (cfg.useHoughCircles) {
        drawConfigFloat("houghDp", reinterpret_cast<float&>(cfg.houghDp), 1.f, 3.f);
        drawConfigFloat("houghMinDist", reinterpret_cast<float&>(cfg.houghMinDist), 10.f, 200.f);
        drawConfigFloat("houghParam1", reinterpret_cast<float&>(cfg.houghParam1), 10.f, 200.f);
        drawConfigFloat("houghParam2", reinterpret_cast<float&>(cfg.houghParam2), 5.f, 80.f);
        drawConfigInt("houghMinRadius", cfg.houghMinRadius, 5, 300);
        drawConfigInt("houghMaxRadius", cfg.houghMaxRadius, 10, 500);
    }
    drawConfigFloat("minContourArea", reinterpret_cast<float&>(cfg.minContourArea), 10.f, 2000.f);
    drawConfigFloat("nestedOutlineKeepMinRatio",
                    reinterpret_cast<float&>(cfg.nestedOutlineKeepMinRatio), 0.01f, 0.5f);
    drawConfigFloat("nestedBlobSuppressMaxRatio",
                    reinterpret_cast<float&>(cfg.nestedBlobSuppressMaxRatio), 0.05f, 0.6f);
    drawConfigInt("minBboxSide", cfg.minBboxSide, 1, 50);
    drawConfigFloat("maxContourAreaRatio",
                    reinterpret_cast<float&>(cfg.maxContourAreaRatio), 0.3f, 0.99f);
    drawConfigFloat("contourDedupeIou", reinterpret_cast<float&>(cfg.contourDedupeIou),
                    0.1f, 0.95f);
}

void drawClassificationConfig(infra::AppConfig& cfg) {
    if (!ImGui::CollapsingHeader("Классификация форм", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    drawConfigFloat("minClassificationConfidence",
                    reinterpret_cast<float&>(cfg.minClassificationConfidence), 0.05f, 0.95f);
    drawConfigFloat("contourApproxEpsilon",
                    reinterpret_cast<float&>(cfg.contourApproxEpsilon), 0.005f, 0.08f);
}

void drawNoiseFreeConfig(infra::AppConfig& cfg) {
    if (!ImGui::CollapsingHeader("Фильтр шума (noiseFree)")) {
        return;
    }
    drawConfigInt("bilateralDiameter", cfg.bilateralDiameter, 0, 15);
    drawConfigFloat("gaussianSigma", reinterpret_cast<float&>(cfg.gaussianSigma), 0.f, 5.f);
    drawConfigInt("medianKernel", cfg.medianKernel, 0, 15);
    drawConfigInt("jpegQuality", cfg.jpegQuality, 50, 100);
}

void drawUi(GuiState& state) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("visionCore", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                     | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Справка")) {
            if (ImGui::MenuItem("О программе")) {
                state.showLog = false;
                state.statusLine =
                    u8"visionCore — курсовой проект. CLI: visionCore_app.exe <путь> [опции]";
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::TextUnformatted(u8"Входные данные");
    ImGui::InputText("##path", state.inputPath, sizeof(state.inputPath));
    ImGui::SameLine();
    if (ImGui::Button(u8"Файл")) {
        std::string picked;
        if (pickImageFile(picked)) {
            std::snprintf(state.inputPath, sizeof(state.inputPath), "%s", picked.c_str());
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(u8"Папка")) {
        std::string picked;
        if (pickFolder(picked)) {
            std::snprintf(state.inputPath, sizeof(state.inputPath), "%s", picked.c_str());
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted(u8"Создавать файлы");
    ImGui::Checkbox(u8"*-shapeOutline", &state.outputs.saveOutline);
    ImGui::SameLine();
    ImGui::Checkbox(u8"*-noiseFree", &state.outputs.saveNoiseFree);
    ImGui::SameLine();
    ImGui::Checkbox(u8"*-report.txt", &state.outputs.saveReport);

    ImGui::Separator();
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

    ImGui::SameLine();
    if (ImGui::Button(u8"Журнал")) {
        state.showLog = !state.showLog;
    }

    ImGui::Separator();
    if (state.runState == RunState::Done) {
        ImGui::TextColored(ImVec4(0.1f, 0.7f, 0.2f, 1.f), u8"Работа завершена.");
    }
    ImGui::TextWrapped("%s", state.statusLine.c_str());

    if (state.showLog && !state.lastLog.empty()) {
        ImGui::Begin(u8"Журнал обработки", &state.showLog);
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
