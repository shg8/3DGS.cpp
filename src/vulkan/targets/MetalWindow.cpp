#include "MetalWindow.h"

MetalWindow::MetalWindow(void* caMetalLayer, int width, int height) : caMetalLayer(caMetalLayer), width(width), height(height) {
}

VkSurfaceKHR MetalWindow::createSurface(std::shared_ptr<VulkanContext> context) {
    this->context = context;
    vk::MetalSurfaceCreateInfoEXT surfaceCreateInfo {{}, caMetalLayer};
    return context->instance->createMetalSurfaceEXT(surfaceCreateInfo);
}

std::array<bool, 3> MetalWindow::getMouseButton() {
    return Window::getMouseButton();
}

std::vector<std::string> MetalWindow::getRequiredInstanceExtensions() {
    return {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_METAL_SURFACE_EXTENSION_NAME};
}

std::pair<uint32_t, uint32_t> MetalWindow::getFramebufferSize() const {
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

std::array<double, 2> MetalWindow::getCursorTranslation() {
    auto result = std::array<double, 2>{accumulatedX, accumulatedY};
    accumulatedX = 0;
    accumulatedY = 0;
    return result;
}

std::array<bool, 7> MetalWindow::getKeys() {
    return Window::getKeys();
}

void MetalWindow::mouseCapture(bool capture) {
    Window::mouseCapture(capture);
}

bool MetalWindow::tick() {
    return Window::tick();
}

void MetalWindow::logTranslation(float x, float y) {
    accumulatedX += x;
    accumulatedY += y;
}
