#ifndef WINDOW_H
#define WINDOW_H
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanContext.h"

class Window {
public:
    virtual VkSurfaceKHR createSurface(std::shared_ptr<VulkanContext> context) = 0;

    virtual std::array<bool, 3> getMouseButton() { return {false, false, false}; }

    virtual std::vector<std::string> getRequiredInstanceExtensions() = 0;

    [[nodiscard]] virtual std::pair<uint32_t, uint32_t> getFramebufferSize() const = 0;

    virtual std::array<double, 2> getCursorTranslation() { return {0, 0}; }

    virtual std::array<bool, 7> getKeys() { return {false, false, false, false, false, false, false}; }

    virtual void mouseCapture(bool capture) { }

    virtual bool tick() { return false; };

    virtual void logTranslation(float x, float y) { };

    virtual void logMovement(float x, float y) { };

    virtual ~Window() = default;

};



#endif //WINDOW_H
