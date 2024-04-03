#ifndef WINDOW_H
#define WINDOW_H
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Swapchain.h"
#include "VulkanContext.h"

class RenderTarget : public Swapchain {
public:
    virtual std::array<bool, 3> getMouseButton() { return {false, false, false}; }

    virtual std::vector<std::string> getRequiredInstanceExtensions() { return {}; };
    virtual std::vector<std::string> getRequiredDeviceExtensions() { return {}; };

    virtual std::array<double, 2> getCursorTranslation() { return {0, 0}; }

    virtual std::array<bool, 7> getKeys() { return {false, false, false, false, false, false, false}; }

    virtual void mouseCapture(bool capture) { }

    virtual bool tick() { return false; };

    virtual void logTranslation(float x, float y) { };

    virtual void logMovement(float x, float y) { };

    virtual std::optional<vk::PhysicalDevice> requirePhysicalDevice(vk::Instance instance) {
        return std::nullopt;
    }
};


#endif //WINDOW_H
