#include "Window.h"
#include <glm/ext/matrix_transform.hpp>

Window::Window(std::string name, int width, int height) {

}

VkSurfaceKHR Window::createSurface(std::shared_ptr<VulkanContext> context) {
    return nullptr;
}

std::array<bool, 3> Window::getMouseButton() {
    return {false, false, false};
}

std::vector<std::string> Window::getRequiredInstanceExtensions() {
    return {};
}

std::pair<uint32_t, uint32_t> Window::getFramebufferSize() const {
    return {0, 0};
}

std::array<double, 2> Window::getCursorTranslation() {
    return {0, 0};
}

std::array<bool, 7> Window::getKeys() {
    return {false, false, false, false, false, false, false};
}

void Window::mouseCapture(bool capture) {
    return;
}

bool Window::tick() {
    return false;
}
