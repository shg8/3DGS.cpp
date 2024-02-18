#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include "VulkanContext.h"
#include "Swapchain.h"
#include "Window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

class ImguiManager {
public:
    ImguiManager(std::shared_ptr<VulkanContext> context, std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Window> window);

    void createCommandPool();

    void setStyle();

    void init();
    void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

    void draw(vk::CommandBuffer commandBuffer, uint32_t currentImageIndex, std::function<void(void)> imguiFunction);

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<Window> window;
    vk::UniqueCommandPool commandPool;
    vk::UniqueCommandBuffer commandBuffer;
    vk::UniqueFence fence;
    vk::UniqueDescriptorPool descriptorPool;
};



#endif //IMGUIMANAGER_H
