#ifndef OPENXRSTEREO_H
#define OPENXRSTEREO_H
#include <3dgs.h>
#include <string>
#include <vector>

#include "../RenderTarget.h"
#include "../Swapchain.h"

class OpenXRStereo : public RenderTarget {
public:
    explicit OpenXRStereo(const VulkanSplatting::OpenXRConfiguration &configuration)
        : configuration(configuration) {
    }

    [[nodiscard]] vk::Extent2D currentExtent() const override;

    std::pair<std::optional<uint32_t>, bool> acquireNextImage() override;

    bool present(const std::vector<vk::Semaphore> &waitSemaphores, uint32_t uint32) override;

    std::array<bool, 3> getMouseButton() override;

    std::vector<std::string> getRequiredInstanceExtensions() override;

    std::vector<std::string> getRequiredDeviceExtensions() override;

    std::array<double, 2> getCursorTranslation() override;

    std::array<bool, 7> getKeys() override;

    void mouseCapture(bool capture) override;

    bool tick() override;

    void logTranslation(float x, float y) override;

    void logMovement(float x, float y) override;

    std::optional<vk::PhysicalDevice> requirePhysicalDevice(vk::Instance instance) override;

    virtual void postVulkanInit(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, uint32_t queueFamily, uint32_t queueIndex);


private:
    VulkanSplatting::OpenXRConfiguration configuration;
};



#endif //OPENXRSTEREO_H
