#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include <functional>

#include "VulkanContext.h"
#include "Swapchain.h"
#include "RenderTarget.h"

class ImguiManager {
public:
    ImguiManager(std::shared_ptr<VulkanContext> context, std::shared_ptr<RenderTarget> window);

    void createCommandPool();

    void setStyle();

    void init();
    void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImageIndex, std::function<void(void)> imguiFunction);

    ~ImguiManager();

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<RenderTarget> window;
    vk::UniqueCommandPool commandPool;
    vk::UniqueCommandBuffer commandBuffer;
    vk::UniqueFence fence;
    vk::UniqueDescriptorPool descriptorPool;
};



#endif //IMGUIMANAGER_H
