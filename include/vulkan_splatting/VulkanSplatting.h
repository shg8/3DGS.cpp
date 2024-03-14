#ifndef VULKANSPLATTING_H
#define VULKANSPLATTING_H

#include <optional>
#include <string>

class VulkanSplatting {
public:
    struct RendererConfiguration {
        bool enableVulkanValidationLayers = false;
        std::optional<uint8_t> physicalDeviceId = std::nullopt;
        bool immediateSwapchain = false;
        std::string scene;

        float fov = 45.0f;
        float near = 0.2f;
        float far = 1000.0f;
        bool enableGui = true;
        uint32_t width = 800;
        uint32_t height = 600;
    };

    explicit VulkanSplatting(RendererConfiguration configuration) : configuration(configuration) {}

    void start() const;
private:
    RendererConfiguration configuration;
};

#endif //VULKANSPLATTING_H
