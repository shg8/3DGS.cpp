#include "GUIManager.h"

#include <iostream>

#include "imgui.h"
#include "implot/implot.h"

struct ScrollingBuffer {
    int maxSize;
    int offset;
    ImVector<ImVec2> data;
    explicit ScrollingBuffer(const int max_size = 10000) {
        maxSize = max_size;
        offset  = 0;
        data.reserve(maxSize);
    }
    void addPoint(float x, float y) {
        if (data.size() < maxSize)
            data.push_back(ImVec2(x,y));
        else {
            data[offset] = ImVec2(x,y);
            offset =  (offset + 1) % maxSize;
        }
    }
    void clear() {
        if (data.size() > 0) {
            data.shrink(0);
            offset  = 0;
        }
    }
};

static std::shared_ptr<std::unordered_map<std::string, ScrollingBuffer>> metricsMap;
static std::shared_ptr<std::unordered_map<std::string, float>> textMetricsMap;

GUIManager::GUIManager() {
    metricsMap = std::make_shared<std::unordered_map<std::string, ScrollingBuffer>>();
    textMetricsMap = std::make_shared<std::unordered_map<std::string, float>>();
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
            if (!values.data.empty()) {
                ImPlot::PlotLine(name.c_str(), &values.data[0].x, &values.data[0].y, values.data.size(), 0,
                                 values.offset, 2 * sizeof(float));
            }
        }
        ImPlot::EndPlot();
    }
    ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");
    ImGui::End();

    // always auto resize

    bool popen = true;
    ImGui::SetNextWindowPos(ImVec2(10, 270), ImGuiCond_FirstUseEver);
    ImGui::Begin("Metrics", &popen, ImGuiWindowFlags_AlwaysAutoResize);
    for (auto& [name, value]: *textMetricsMap) {
        ImGui::Text("%s: %.2f", name.c_str(), value);
    }
    for (auto & [name, values]: *metricsMap) {
        ImGui::Text("%s: %.2f", name.c_str(), values.data.empty() ? 0 : values.data.back().y);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(10, 310), ImGuiCond_FirstUseEver);
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

void GUIManager::pushTextMetric(const std::string& name, float value) {
    if (!textMetricsMap->contains(name)) {
        textMetricsMap->insert({name, value});
    } else {
        textMetricsMap->at(name) = value;
    }
}

void GUIManager::pushMetric(const std::string& name, float value) {
    int maxSize = 600;
    if (!metricsMap->contains(name)) {
        metricsMap->insert({name, ScrollingBuffer{}});
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
