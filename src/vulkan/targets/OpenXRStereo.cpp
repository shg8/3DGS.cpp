#ifdef VKGS_ENABLE_OPENXR
#include "OpenXRStereo.h"
VkSurfaceKHR OpenXRStereo::createSurface(std::shared_ptr<VulkanContext> context) {
}

std::array<bool, 3> OpenXRStereo::getMouseButton() {
    return RenderTarget::getMouseButton();
}

std::vector<std::string> OpenXRStereo::getRequiredInstanceExtensions() {
    return configuration.instanceExtensions;
}

std::vector<std::string> OpenXRStereo::getRequiredDeviceExtensions() {
    return configuration.deviceExtensions;
}

std::pair<uint32_t, uint32_t> OpenXRStereo::getFramebufferSize() const {
    return {};
}

std::array<double, 2> OpenXRStereo::getCursorTranslation() {
    return RenderTarget::getCursorTranslation();
}

std::array<bool, 7> OpenXRStereo::getKeys() {
    return RenderTarget::getKeys();
}

void OpenXRStereo::mouseCapture(bool capture) {
    RenderTarget::mouseCapture(capture);
}

bool OpenXRStereo::tick() {
    return RenderTarget::tick();
}

void OpenXRStereo::logTranslation(float x, float y) {
    RenderTarget::logTranslation(x, y);
}

void OpenXRStereo::logMovement(float x, float y) {
    RenderTarget::logMovement(x, y);
}

std::optional<vk::PhysicalDevice> OpenXRStereo::requirePhysicalDevice(vk::Instance instance) {
    void *pdPtr = configuration.getPhysicalDevice(instance);
    return {static_cast<vk::PhysicalDevice>(static_cast<VkPhysicalDevice>(pdPtr))};
}

void OpenXRStereo::postVulkanInit(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device,
    uint32_t queueFamily, uint32_t queueIndex) {
    configuration.postVulkanInit(instance, physicalDevice, device, queueFamily, queueIndex);
}
#endif