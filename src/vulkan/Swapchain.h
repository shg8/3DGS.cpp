#ifndef VULKAN_SPLATTING_SWAPCHAIN_H
#define VULKAN_SPLATTING_SWAPCHAIN_H


#include <memory>
#include "VulkanContext.h"
#include "Window.h"

class Swapchain {
public:
    Swapchain(const std::shared_ptr<VulkanContext> &context, const std::shared_ptr<Window> &window, bool immediate);

    vk::UniqueSwapchainKHR swapchain;
    vk::Extent2D swapchainExtent;
    std::vector<std::shared_ptr<Image>> swapchainImages;
    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::Format swapchainFormat;
    vk::PresentModeKHR presentMode;
    uint32_t imageCount;

    void recreate();
private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<Window> window;

    bool immediate = false;

    void createSwapchain();

    void createSwapchainImages();
};


#endif //VULKAN_SPLATTING_SWAPCHAIN_H
