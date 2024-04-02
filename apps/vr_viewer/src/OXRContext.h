
#ifndef OXRCONTEXT_H
#define OXRCONTEXT_H
#include <openxr/openxr.h>
#include <vulkan/vulkan.hpp>

#include "OXRUtils.h"


class OXRContext {
public:
    void setup();
    void* getPhysicalDevice(void *instance) const;
private:
    XrInstance oxrInstance;
    XrSystemId systemId;
    XrViewConfigurationView views[2] = {{XR_TYPE_VIEW_CONFIGURATION_VIEW}, {XR_TYPE_VIEW_CONFIGURATION_VIEW}};

    void setupOXR();

    void createInstance();

    void createSystem();

    void setupViews();

    [[nodiscard]] std::vector<std::string> getRequiredVulkanInstanceExtensions() const;

    [[nodiscard]] std::vector<std::string> getRequiredVulkanDeviceExtensions() const;
};



#endif //OXRCONTEXT_H
