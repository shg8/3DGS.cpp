#ifndef LAYER_H
#define LAYER_H
#include <cstdint>
#include <memory>
#include <vector>

#include "OXRContext.h"
#include <openxr/openxr_platform.h>

namespace OXR {

class Layer {
public:
    explicit Layer(std::shared_ptr<OXRContext> context, XrCompositionLayerFlags flags, bool depth_enabled);


private:
    XrCompositionLayerProjection layer = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    XrCompositionLayerDepthInfoKHR depthLayer = {XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR};
    XrCompositionLayerProjectionView views[2] = {{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}, {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}};
    XrSwapchain swapchains[2] = {XR_NULL_HANDLE, XR_NULL_HANDLE};
    uint32_t swapchainSize[2] = {0, 0};
    uint32_t lastImageAcquired[2] = {0, 0};

    bool depthEnabled = false;
    XrSwapchain depthSwapchains[2] = {XR_NULL_HANDLE, XR_NULL_HANDLE};
    uint32_t depthSwapchainSize[2] = {0, 0};
    uint32_t lastDepthImageAcquired[2] = {0, 0};

    std::vector<std::array<XrSwapchainImageVulkanKHR, 2>> images;
    std::vector<std::array<XrSwapchainImageVulkanKHR, 2>> depthImages;

    void createSwapchains(std::shared_ptr<OXRContext> context);
    void createProjectionViews(std::shared_ptr<OXRContext> context, uint64_t flags);
};

} // OXR

#endif //LAYER_H
