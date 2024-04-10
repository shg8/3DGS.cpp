#ifdef VKGS_ENABLE_OPENXR
#include "OpenXRStereo.h"

vk::Extent2D OpenXRStereo::currentExtent() const {
    auto [width, height] = configuration.getCurrentExtent();
    assert(width > 0 && height > 0);
    return {width, height};
}

std::pair<std::optional<uint32_t>, bool> OpenXRStereo::acquireNextImage() {

}

bool OpenXRStereo::present(const std::vector<vk::Semaphore> &waitSemaphores, uint32_t uint32) {
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