#ifndef MANAGEDSWAPCHAIN_H
#define MANAGEDSWAPCHAIN_H
#include "../RenderTarget.h"


class ManagedSwapchain : public RenderTarget {
public:
    explicit ManagedSwapchain(bool immediate);

    void setup(std::shared_ptr<VulkanContext> context);

    virtual VkSurfaceKHR createSurface(std::shared_ptr<VulkanContext> context) = 0;

    [[nodiscard]] virtual vk::Extent2D currentExtent() const;

    virtual std::pair<std::optional<uint32_t>, bool> acquireNextImage();

    virtual bool present(const std::vector<vk::Semaphore> &waitSemaphores, uint32_t uint32);

private:
    std::shared_ptr<VulkanContext> context;

    vk::UniqueSwapchainKHR swapchain;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D swapchainExtent;
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

    bool immediate = false;

    void createSwapchain();

    void createSwapchainImages();

    bool recreateSwapchain();

protected:
    virtual std::pair<uint32_t, uint32_t> getFramebufferSize() = 0;
};



#endif //MANAGEDSWAPCHAIN_H
