#include "VRViewer.h"

#include <memory>

#include "OXRContext.h"
#include "3dgs.h"

void VRViewer::run() {
    context = std::make_shared<OXRContext>();
    context->setup();

    VulkanSplatting::OpenXRConfiguration configuration;
    configuration.getPhysicalDevice = std::bind(&OXRContext::getPhysicalDevice, context.get(), std::placeholders::_1);
}

