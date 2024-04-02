#include "VRViewer.h"

#include <memory>
#include <openxr/openxr_platform.h>

#include "OXRContext.h"
#include "3dgs.h"
#include "spdlog/spdlog.h"

void VRViewer::run() {
    context = std::make_shared<OXRContext>();
    context->setup();

    VulkanSplatting::OpenXRConfiguration configuration;
    configuration.getPhysicalDevice = std::bind(&OXRContext::getPhysicalDevice, context.get(), std::placeholders::_1);
}

void VRViewer::finishSetup(void *vkInstance, void *vkPhysicalDevice, void *vkDevice, uint32_t vkQueueFamilyIndex,
    uint32_t vkQueueIndex) {
    context->createSession(vkInstance, vkPhysicalDevice, vkDevice, vkQueueFamilyIndex, vkQueueIndex);
    context->createReferenceSpace();
    context->beginSession();
}

