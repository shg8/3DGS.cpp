
#include "OXRContext.h"

#include "vulkan/vulkan.hpp"

#include <stdexcept>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "spdlog/spdlog.h"

void OXRContext::setup() {
    bool vulkanSupported = OXR::isExtensionSupported(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
    if (!vulkanSupported) {
        throw std::runtime_error("Vulkan not supported");
    }

    createInstance();
    createSystem();
    setupViews();
    std::ignore = getRequiredVulkanInstanceExtensions();
    std::ignore = getRequiredVulkanDeviceExtensions();
}

void OXRContext::createInstance() {
    std::vector<std::string> enabledExtensions;
    enabledExtensions.push_back(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);

    std::vector<std::string> enabledLayers;
    // enabledLayers.push_back("XR_APILAYER_LUNARG_core_validation");

    OXR::CharArrayWrapper enabledExtensionsWrapper(enabledExtensions);
    OXR::CharArrayWrapper enabledLayersWrapper(enabledLayers);
    XrInstanceCreateInfo createInfo = {XR_TYPE_INSTANCE_CREATE_INFO};
    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.enabledExtensionNames = enabledExtensionsWrapper.get();
    // createInfo.enabledApiLayerCount = static_cast<uint32_t>(enabledLayers.size());
    // createInfo.enabledApiLayerNames = enabledLayersWrapper.get();
    createInfo.applicationInfo = {"3DGS VR Viewer", 1, "3DGS.cpp", 1, XR_CURRENT_API_VERSION};

    auto result = xrCreateInstance(&createInfo, &oxrInstance);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to create OpenXR instance");
    }

    spdlog::debug("Created OpenXR instance");
}

void OXRContext::createSystem() {
    XrSystemGetInfo systemInfo = {XR_TYPE_SYSTEM_GET_INFO, nullptr, XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY};
    auto result = xrGetSystem(oxrInstance, &systemInfo, &systemId);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get OpenXR system");
    }

    XrSystemProperties systemProperties = {XR_TYPE_SYSTEM_PROPERTIES};
    result = xrGetSystemProperties(oxrInstance, systemId, &systemProperties);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get OpenXR system properties");
    }

    spdlog::debug("OpenXR system properties:");
    spdlog::debug("  systemId: {}", systemId);
    spdlog::debug("  vendorId: {}", systemProperties.vendorId);
    spdlog::debug("  systemName: {}", systemProperties.systemName);
    spdlog::debug("  graphicsProperties:");
    spdlog::debug("    maxLayerCount: {}", systemProperties.graphicsProperties.maxLayerCount);
    spdlog::debug("    maxSwapchainImageHeight: {}", systemProperties.graphicsProperties.maxSwapchainImageHeight);
    spdlog::debug("    maxSwapchainImageWidth: {}", systemProperties.graphicsProperties.maxSwapchainImageWidth);
    spdlog::debug("  trackingProperties:");
    spdlog::debug("    orientationTracking: {}", systemProperties.trackingProperties.orientationTracking);
    spdlog::debug("    positionTracking: {}", systemProperties.trackingProperties.positionTracking);
}

void OXRContext::setupViews() {
    uint32_t viewCount;
    auto result = xrEnumerateViewConfigurationViews(oxrInstance, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to enumerate view configuration views");
    }

    if (viewCount != 2) {
        throw std::runtime_error("Expected 2 views");
    }

    result = xrEnumerateViewConfigurationViews(oxrInstance, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, views);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to enumerate view configuration views");
    }
}

std::vector<std::string> OXRContext::getRequiredVulkanInstanceExtensions() const {
    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR;
    auto result = xrGetInstanceProcAddr(oxrInstance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanInstanceExtensionsKHR));
     if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get xrGetVulkanInstanceExtensionsKHR");
    }

    uint32_t size;
    result = xrGetVulkanInstanceExtensionsKHR(oxrInstance, systemId, 0, &size, nullptr);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get Vulkan instance extensions size");
    }

    char* extensions = new char[size];
    result = xrGetVulkanInstanceExtensionsKHR(oxrInstance, systemId, size, &size, extensions);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get Vulkan instance extensions");
    }

    spdlog::debug("Vulkan instance extensions: {}", extensions);

    // split extensions by space
    std::vector<std::string> extensionList;
    std::string extension;
    for (size_t i = 0; i < size; ++i) {
        if (extensions[i] == ' ') {
            extensionList.push_back(extension);
            extension.clear();
        } else {
            extension.push_back(extensions[i]);
        }
    }

    return extensionList;
}

std::vector<std::string> OXRContext::getRequiredVulkanDeviceExtensions() const {
    PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR;
    auto result = xrGetInstanceProcAddr(oxrInstance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanDeviceExtensionsKHR));
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get xrGetVulkanDeviceExtensionsKHR");
    }

    uint32_t size;
    result = xrGetVulkanDeviceExtensionsKHR(oxrInstance, systemId, 0, &size, nullptr);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get Vulkan device extensions size");
    }

    char* extensions = new char[size];
    result = xrGetVulkanDeviceExtensionsKHR(oxrInstance, systemId, size, &size, extensions);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get Vulkan device extensions");
    }

    spdlog::debug("Vulkan device extensions: {}", extensions);

    // split extensions by space
    std::vector<std::string> extensionList;
    std::string extension;
    for (size_t i = 0; i < size; ++i) {
        if (extensions[i] == ' ') {
            extensionList.push_back(extension);
            extension.clear();
        } else {
            extension.push_back(extensions[i]);
        }
    }

    return extensionList;
}

void * OXRContext::getPhysicalDevice(void *instance) const {
    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR;
    auto result = xrGetInstanceProcAddr(oxrInstance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanGraphicsDeviceKHR));
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get xrGetVulkanGraphicsDeviceKHR");
    }

    VkPhysicalDevice physicalDevice;
    result = xrGetVulkanGraphicsDeviceKHR(oxrInstance, systemId, static_cast<VkInstance>(instance), &physicalDevice);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to get Vulkan graphics device");
    }

    return physicalDevice;
}

void OXRContext::createSession(void *vkInstance, void *vkPhysicalDevice, void *vkDevice, uint32_t vkQueueFamilyIndex,
    uint32_t vkQueueIndex) {
    XrGraphicsBindingVulkanKHR graphicsBinding = {XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR};
    graphicsBinding.instance = static_cast<VkInstance>(vkInstance);
    graphicsBinding.physicalDevice = static_cast<VkPhysicalDevice>(vkPhysicalDevice);
    graphicsBinding.device = static_cast<VkDevice>(vkDevice);
    graphicsBinding.queueFamilyIndex = vkQueueFamilyIndex;
    graphicsBinding.queueIndex = vkQueueIndex;

    XrSessionCreateInfo createInfo = {XR_TYPE_SESSION_CREATE_INFO};
    createInfo.next = &graphicsBinding;
    createInfo.systemId = systemId;

    auto result = xrCreateSession(oxrInstance, &createInfo, &oxrSession);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to create OpenXR session");
    }

    spdlog::debug("Created OpenXR session");
}

void OXRContext::createReferenceSpace() {
    XrPosef identityPose = {{0, 0, 0, 1}, {0, 0, 0}};
    XrReferenceSpaceCreateInfo createInfo = {XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    createInfo.poseInReferenceSpace = identityPose;
    createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;

    auto result = xrCreateReferenceSpace(oxrSession, &createInfo, &localSpace);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to create OpenXR reference space");
    }
}

void OXRContext::beginSession() {
    XrSessionBeginInfo beginInfo = {XR_TYPE_SESSION_BEGIN_INFO};
    beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    auto result = xrBeginSession(oxrSession, &beginInfo);
    if (XR_FAILED(result)) {
        throw std::runtime_error("Failed to begin OpenXR session");
    }
}
