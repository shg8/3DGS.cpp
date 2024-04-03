#ifndef VULKANSPLATTING_H
#define VULKANSPLATTING_H

#include <functional>
#include <optional>
#include <string>
#include <memory>
#include <vector>

class RenderTarget;
class Renderer;

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
        bool enableGui = false;

        std::shared_ptr<RenderTarget> renderingTarget;
    };

    explicit VulkanSplatting(RendererConfiguration configuration) : configuration(configuration) {}

#ifdef VKGS_ENABLE_GLFW
    static std::shared_ptr<RenderTarget> createGlfwWindow(std::string name, int width, int height);
#endif

#ifdef VKGS_ENABLE_METAL
    static std::shared_ptr<RenderTarget> createMetalWindow(void *caMetalLayer, int width, int height);
#endif

#ifdef VKGS_ENABLE_OPENXR
    struct OpenXRConfiguration {
        std::vector<std::string> instanceExtensions;
        std::vector<std::string> deviceExtensions;

        std::function<void*(void*)> getPhysicalDevice;
        std::function<void(void*, void*, void*, uint32_t, uint32_t)> postVulkanInit;
    };
    static std::shared_ptr<RenderTarget> createOpenXRRenderTarget(OpenXRConfiguration configuration);
#endif

    void start();

    void initialize();

    void draw();

    void logTranslation(float x, float y);

    void logMovement(float x, float y, float z);

    void stop();
private:
    RendererConfiguration configuration;
    std::shared_ptr<Renderer> renderer;
};

#endif //VULKANSPLATTING_H
