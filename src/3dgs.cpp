#include "3dgs.h"
#include "Renderer.h"
#include "vulkan/targets/OpenXRStereo.h"

#ifdef VKGS_ENABLE_GLFW
#include "vulkan/targets/GLFWWindow.h"
std::shared_ptr<RenderingTarget> VulkanSplatting::createGlfwWindow(std::string name, int width, int height) {
    return std::make_shared<GLFWWindow>(name, width, height);
}
#endif

#ifdef VKGS_ENABLE_METAL
#include "vulkan/windowing/MetalWindow.h"
std::shared_ptr<RenderingTarget> VulkanSplatting::createMetalWindow(void *caMetalLayer, int width, int height) {
    return std::make_shared<MetalWindow>(caMetalLayer, width, height);
}
#endif

std::shared_ptr<RenderingTarget> VulkanSplatting::createOpenXRRenderingTarget(OpenXRConfiguration configuration) {
    auto target = std::make_shared<OpenXRStereo>(configuration);
    return target;
}

void VulkanSplatting::start() {
    // Create the renderer
    renderer = std::make_shared<Renderer>(configuration);
    renderer->initialize();
    renderer->run();
}

void VulkanSplatting::initialize() {
    renderer = std::make_shared<Renderer>(configuration);
    renderer->initialize();
}

void VulkanSplatting::draw() {
    renderer->draw();
}

void VulkanSplatting::logTranslation(float x, float y) {
    configuration.renderingTarget->logTranslation(x, y);
}

void VulkanSplatting::logMovement(float x, float y, float z) {
    renderer->camera.translate(glm::vec3(x, y, z));
}

void VulkanSplatting::stop() {
    renderer->stop();
}
