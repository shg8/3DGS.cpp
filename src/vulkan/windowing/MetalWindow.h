#ifndef METALWINDOW_H
#define METALWINDOW_H
#include "../Window.h"
#include "../VulkanContext.h"

class MetalWindow final : public Window {
public:
    MetalWindow(void *caMetalLayer, int width, int height);

    VkSurfaceKHR createSurface(std::shared_ptr<VulkanContext> context) override;

    std::array<bool, 3> getMouseButton() override;

    std::vector<std::string> getRequiredInstanceExtensions() override;

    [[nodiscard]] std::pair<uint32_t, uint32_t> getFramebufferSize() const override;

    std::array<double, 2> getCursorTranslation() override;

    std::array<bool, 7> getKeys() override;

    void mouseCapture(bool capture) override;

    void logTranslation(float x, float y) override;

    bool tick() override;
private:
    void *caMetalLayer;
    int width = 0;
    int height = 0;
    std::shared_ptr<VulkanContext> context;

    float accumulatedX = 0;
    float accumulatedY = 0;
};



#endif //METALWINDOW_H
