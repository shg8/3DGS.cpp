#ifndef GUIMANAGER_H
#define GUIMANAGER_H
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include "imgui.h"
#include "implot/implot.h"

struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void addPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void clear() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

class GUIManager {
public:
    GUIManager();

    static void init();

    static void buildGui();

    static void pushMetric(const std::string& name, float value);

    static void pushMetric(const std::unordered_map<std::string, float>& name);

};

#endif //GUIMANAGER_H
