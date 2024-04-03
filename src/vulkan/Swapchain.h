#ifndef VULKAN_SPLATTING_SWAPCHAIN_H
#define VULKAN_SPLATTING_SWAPCHAIN_H


#include <memory>
#include "VulkanContext.h"
#include "RenderTarget.h"

class Swapchain {
public:
    Swapchain(const std::shared_ptr<VulkanContext> &context, const std::shared_ptr<RenderTarget> &window, bool immediate);

    [[nodiscard]] virtual vk::Extent2D currentExtent() const {
        return swapchainExtent;
    }

    std::pair<std::optional<uint32_t>, bool> acquireNextImage();

    bool present(const std::vector<vk::Semaphore> &waitSemaphores, uint32_t uint32);

    std::vector<std::shared_ptr<Image>> swapchainImages;
    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    vk::Format swapchainFormat;
    uint32_t imageCount;

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<RenderTarget> window;

    vk::UniqueSwapchainKHR swapchain;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D swapchainExtent;
    vk::PresentModeKHR presentMode;

    bool immediate = false;

    void createSwapchain();

    void createSwapchainImages();

    bool recreateSwapchain();

};


#endif //VULKAN_SPLATTING_SWAPCHAIN_H
