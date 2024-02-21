#ifndef WINDOW_H
#define WINDOW_H
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanContext.h"


class Window {
public:
    Window(std::string name, int width, int height);

    VkSurfaceKHR createSurface(std::shared_ptr<VulkanContext> context);

    static std::vector<std::string> getRequiredInstanceExtensions();

    std::pair<uint32_t, uint32_t> getFramebufferSize() const;

    std::array<double, 2> getCursorTranslation();

    std::array<bool, 4> getKeys();

    bool tick();

    void* window;

private:
    VkSurfaceKHR surface;

    double lastX = 0.0;
    double lastY = 0.0;
};



#endif //WINDOW_H
