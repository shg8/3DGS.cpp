#ifndef GUIMANAGER_H
#define GUIMANAGER_H
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include "../cmake-build-debug/_deps/imgui-src/imgui.h"
#include "../third_party/implot/implot.h"

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

class GUIManager {
public:
    GUIManager();

    static void init();

    void buildGui();

    static void pushTextMetric(const std::string& name, float value);

    static void pushMetric(const std::string& name, float value);

    static void pushMetric(const std::unordered_map<std::string, float>& name);

    static bool wantCaptureMouse();

    static bool wantCaptureKeyboard();

    bool mouseCapture = false;

};

#endif //GUIMANAGER_H
