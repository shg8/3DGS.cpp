#include "OpenXRStereo.h"

VkSurfaceKHR OpenXRStereo::createSurface(std::shared_ptr<VulkanContext> context) {
}

std::array<bool, 3> OpenXRStereo::getMouseButton() {
    return RenderingTarget::getMouseButton();
}

std::vector<std::string> OpenXRStereo::getRequiredInstanceExtensions() {
}

std::pair<uint32_t, uint32_t> OpenXRStereo::getFramebufferSize() const {
}

std::array<double, 2> OpenXRStereo::getCursorTranslation() {
    return RenderingTarget::getCursorTranslation();
}

std::array<bool, 7> OpenXRStereo::getKeys() {
    return RenderingTarget::getKeys();
}

void OpenXRStereo::mouseCapture(bool capture) {
    RenderingTarget::mouseCapture(capture);
}

bool OpenXRStereo::tick() {
    return RenderingTarget::tick();
}

void OpenXRStereo::logTranslation(float x, float y) {
    RenderingTarget::logTranslation(x, y);
}

void OpenXRStereo::logMovement(float x, float y) {
    RenderingTarget::logMovement(x, y);
}
