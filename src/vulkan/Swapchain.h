#ifndef VULKAN_SPLATTING_SWAPCHAIN_H
#define VULKAN_SPLATTING_SWAPCHAIN_H


#include <memory>
#include "VulkanContext.h"

class Swapchain {
public:
    [[nodiscard]] virtual vk::Extent2D currentExtent() const = 0;

    virtual std::pair<std::optional<uint32_t>, bool> acquireNextImage() = 0;

    virtual bool present(const std::vector<vk::Semaphore> &waitSemaphores, uint32_t uint32) = 0;

    virtual ~Swapchain() = default;

    std::vector<std::shared_ptr<Image>> swapchainImages;
    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    vk::Format swapchainFormat;
    uint32_t imageCount;
};


#endif //VULKAN_SPLATTING_SWAPCHAIN_H
