#include "GUIManager.h"

#include <iostream>

static std::shared_ptr<std::unordered_map<std::string, ScrollingBuffer>> metricsMap;

GUIManager::GUIManager() {
    metricsMap = std::make_shared<std::unordered_map<std::string, ScrollingBuffer>>();
}

void GUIManager::init() {
    ImPlot::CreateContext();
}

void GUIManager::buildGui() {
    if (mouseCapture) {
        ImGui::BeginDisabled(true);
    }

    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Performance");

    static float history = 10;

    static ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit;
    if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("s", "time (ms)", flags, flags);
        const auto t = ImGui::GetTime();
        ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
        for (auto& [name, values]: *metricsMap) {
            if (!values.Data.empty()) {
                ImPlot::PlotLine(name.c_str(), &values.Data[0].x, &values.Data[0].y, values.Data.size(), 0,
                                 values.Offset, 2 * sizeof(float));
            }
        }
        ImPlot::EndPlot();
    }
    ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");
    ImGui::End();

    // always auto resize

    bool popen = true;
    ImGui::SetNextWindowPos(ImVec2(10, 270), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls", &popen, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("WASD: move");
    ImGui::Text("Space: up");
    ImGui::Text("Shift: down");
    ImGui::Text("Left click: capture mouse");
    ImGui::Text("ESC: release mouse");
    ImGui::Text("Mouse captured: %s", mouseCapture ? "true" : "false");
    ImGui::End();

    if (mouseCapture) {
        ImGui::EndDisabled();
    }
}

void GUIManager::pushMetric(const std::string& name, float value) {
    int maxSize = 600;
    if (!metricsMap->contains(name)) {
        metricsMap->insert({name, ScrollingBuffer(maxSize)});
    }
    metricsMap->at(name).addPoint(ImGui::GetTime(), value);
}

void GUIManager::pushMetric(const std::unordered_map<std::string, float>& name) {
    for (auto& [n, v]: name) {
        pushMetric(n, v);
    }
}

bool GUIManager::wantCaptureMouse() {
    return ImGui::GetIO().WantCaptureMouse;
}

bool GUIManager::wantCaptureKeyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}
