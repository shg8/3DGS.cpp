#ifndef OPENXRSTEREO_H
#define OPENXRSTEREO_H
#include <string>
#include <vector>

#include "../RenderingTarget.h"

class OpenXRStereo : public RenderingTarget {
public:
    VkSurfaceKHR createSurface(std::shared_ptr<VulkanContext> context) override;

    std::array<bool, 3> getMouseButton() override;

    std::vector<std::string> getRequiredInstanceExtensions() override;

    [[nodiscard]] std::pair<uint32_t, uint32_t> getFramebufferSize() const override;

    std::array<double, 2> getCursorTranslation() override;

    std::array<bool, 7> getKeys() override;

    void mouseCapture(bool capture) override;

    bool tick() override;

    void logTranslation(float x, float y) override;

    void logMovement(float x, float y) override;

    std::vector<std::string> instanceExtensions;
    std::vector<std::string> deviceExtensions;
};



#endif //OPENXRSTEREO_H
