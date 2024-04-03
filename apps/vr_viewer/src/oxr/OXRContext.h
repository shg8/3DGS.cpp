
#ifndef OXRCONTEXT_H
#define OXRCONTEXT_H
#include <openxr/openxr.h>
#include <vulkan/vulkan.hpp>

namespace OXR {
    class OXRContext {
    public:
        void setup();
        void* getPhysicalDevice(void *instance) const;

        void createSession(void *vkInstance, void *vkPhysicalDevice, void *vkDevice, uint32_t vkQueueFamilyIndex,
                           uint32_t vkQueueIndex);

        void createReferenceSpace();

        void beginSession();

        [[nodiscard]] std::vector<std::string> getRequiredVulkanInstanceExtensions() const;

        [[nodiscard]] std::vector<std::string> getRequiredVulkanDeviceExtensions() const;

        XrInstance oxrInstance = XR_NULL_HANDLE;
        XrSystemId systemId = XR_NULL_SYSTEM_ID;
        XrViewConfigurationView views[2] = {{XR_TYPE_VIEW_CONFIGURATION_VIEW}, {XR_TYPE_VIEW_CONFIGURATION_VIEW}};
        XrSession oxrSession = XR_NULL_HANDLE;
        XrSpace localSpace = XR_NULL_HANDLE;

    private:
        void createInstance();

        void createSystem();

        void setupViews();

    };
}

#endif //OXRCONTEXT_H
