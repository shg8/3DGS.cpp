#include "VRViewer.h"

#include <complex>
#include <memory>
#include <openxr/openxr_platform.h>

#include "oxr/OXRContext.h"
#include "3dgs.h"
#include "spdlog/spdlog.h"

using namespace OXR;

void VRViewer::run() {
    context = std::make_shared<OXRContext>();
    context->setup();

    VulkanSplatting::OpenXRConfiguration configuration;
    configuration.instanceExtensions = context->getRequiredVulkanInstanceExtensions();
    configuration.deviceExtensions = context->getRequiredVulkanDeviceExtensions();
    configuration.getPhysicalDevice = std::bind(&OXRContext::getPhysicalDevice, context, std::placeholders::_1);
    configuration.postVulkanInit = std::bind(&VRViewer::finishSetup, this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    configuration.getCurrentExtent = std::bind(&VRViewer::getCurrentExtent, this);
    configuration.acquireNextImage = std::bind(&Layer::acquireNextImage, projectionLayer, std::placeholders::_1);
    configuration.present = std::bind(&Layer::present, projectionLayer, std::placeholders::_1, std::placeholders::_2);

    auto renderingTarget = VulkanSplatting::createOpenXRRenderTarget(configuration);
    VulkanSplatting::RendererConfiguration rendererConfiguration;
    rendererConfiguration.scene = config.scenePath;
    rendererConfiguration.enableGui = false;
    VulkanSplatting vulkanSplatting(rendererConfiguration, renderingTarget);
    vulkanSplatting.initialize();
}

void VRViewer::finishSetup(void *vkInstance, void *vkPhysicalDevice, void *vkDevice, uint32_t vkQueueFamilyIndex,
    uint32_t vkQueueIndex) {
    spdlog::info("Finishing setup");
    context->createSession(vkInstance, vkPhysicalDevice, vkDevice, vkQueueFamilyIndex, vkQueueIndex);
    context->createReferenceSpace();
    context->beginSession();
    createProjectionLayer();
}

std::pair<uint32_t, uint32_t> VRViewer::getCurrentExtent() {
    return {projectionLayer->views[0].subImage.imageRect.extent.width, projectionLayer->views[0].subImage.imageRect.extent.height};
}

void VRViewer::createProjectionLayer() {
    projectionLayer = std::make_shared<Layer>(context, XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT, false);
}

