#ifndef OPENXRSTEREO_H
#define OPENXRSTEREO_H
#include <3dgs.h>
#include <string>
#include <vector>

#include "../RenderingTarget.h"

class OpenXRStereo : public RenderingTarget {
public:
    explicit OpenXRStereo(const VulkanSplatting::OpenXRConfiguration &configuration)
        : configuration(configuration) {
    }

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

    std::optional<vk::PhysicalDevice> requirePhysicalDevice(vk::Instance instance) override;

private:
    VulkanSplatting::OpenXRConfiguration configuration;
};



#endif //OPENXRSTEREO_H
