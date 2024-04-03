#include "VRViewer.h"

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
    configuration.getPhysicalDevice = std::bind(&OXRContext::getPhysicalDevice, context.get(), std::placeholders::_1);
    configuration.postVulkanInit = std::bind(&VRViewer::finishSetup, this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);

    auto renderingTarget = VulkanSplatting::createOpenXRRenderTarget(configuration);
}

void VRViewer::finishSetup(void *vkInstance, void *vkPhysicalDevice, void *vkDevice, uint32_t vkQueueFamilyIndex,
    uint32_t vkQueueIndex) {
    context->createSession(vkInstance, vkPhysicalDevice, vkDevice, vkQueueFamilyIndex, vkQueueIndex);
    context->createReferenceSpace();
    context->beginSession();
    createProjectionLayer();
}

void VRViewer::createProjectionLayer() {

}

