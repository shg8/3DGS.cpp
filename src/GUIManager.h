#ifndef GUIMANAGER_H
#define GUIMANAGER_H
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

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

    bool wantToSnapCamera();

    bool mouseCapture = false;

    bool requestCameraSnap = false;

};

#endif //GUIMANAGER_H
