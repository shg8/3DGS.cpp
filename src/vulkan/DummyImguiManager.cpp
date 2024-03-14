#include "ImguiManager.h"

ImguiManager::ImguiManager(std::shared_ptr<VulkanContext> context, std::shared_ptr<Swapchain> swapchain,
                           std::shared_ptr<Window> window) {
}

void ImguiManager::createCommandPool() {
}

void ImguiManager::setStyle() {

}

void ImguiManager::init() {

}

void ImguiManager::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {

}

void ImguiManager::draw(vk::CommandBuffer commandBuffer, uint32_t currentImageIndex, std::function<void(void)> imguiFunction) {

}

ImguiManager::~ImguiManager() {

}
