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
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Performance");

    static float history = 10;

    static ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit;
    if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("ms", "time", flags, flags);
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

    ImGui::SetNextWindowPos(ImVec2(10, 270), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls");
    ImGui::Text("WASD: Move");
    ImGui::Text("Mouse: Look");
    ImGui::End();

}

void GUIManager::pushMetric(const std::string& name, float value) {
    int maxSize = 600;
    if (!metricsMap->contains(name)) {
        metricsMap->insert({name, ScrollingBuffer(maxSize)});
    }
    metricsMap->at(name).addPoint(ImGui::GetTime(), value / 1000000.0);
}

void GUIManager::pushMetric(const std::unordered_map<std::string, unsigned long long>& name) {
    for (auto& [n, v]: name) {
        pushMetric(n, v);
    }
}
