#include "3dgs.h"
#include "Renderer.h"
#ifdef VKGS_ENABLE_OPENXR
#include "vulkan/targets/OpenXRStereo.h"
#endif

#ifdef VKGS_ENABLE_GLFW
#include "vulkan/targets/GLFWWindow.h"

VulkanSplatting::VulkanSplatting(RendererConfiguration configuration,
                                 std::shared_ptr<RenderTarget> renderTarget) : configuration(configuration),
                                                                               renderTarget(renderTarget),
                                                                               renderer(std::make_shared<Renderer>(
                                                                                   configuration, renderTarget)) {

}

std::shared_ptr<RenderTarget> VulkanSplatting::createGlfwWindow(std::string name, int width, int height, bool immediate, bool fixedSize) {
    return std::make_shared<GLFWWindow>(name, width, height, immediate, fixedSize);
}
#endif

#ifdef VKGS_ENABLE_METAL
#include "vulkan/windowing/MetalWindow.h"
std::shared_ptr<RenderTarget> VulkanSplatting::createMetalWindow(void *caMetalLayer, int width, int height) {
    return std::make_shared<MetalWindow>(caMetalLayer, width, height);
}
#endif

#ifdef VKGS_ENABLE_OPENXR
std::shared_ptr<RenderTarget> VulkanSplatting::createOpenXRRenderTarget(OpenXRConfiguration configuration) {
    auto target = std::make_shared<OpenXRStereo>(configuration);
    return target;
}
#endif

void VulkanSplatting::start() {
    // Create the renderer
    renderer->initialize();
    renderer->run();
}

void VulkanSplatting::initialize() {
    renderer->initialize();
}

void VulkanSplatting::draw() {
    renderer->draw();
}

void VulkanSplatting::logTranslation(float x, float y) {
    if (auto rt = renderTarget.lock())
        rt->logTranslation(x, y);
}

void VulkanSplatting::logMovement(float x, float y, float z) {
    renderer->camera.translate(glm::vec3(x, y, z));
}

void VulkanSplatting::stop() {
    renderer->stop();
}
