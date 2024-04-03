#ifndef OPENXRVIEWER_H
#define OPENXRVIEWER_H
#include <args.hxx>
#include <barrier>
#include <memory>
#include <utility>

#include "oxr/OXRContext.h"

class VRViewer {
public:
    struct Configuration {
        std::string scenePath;
    };

    VRViewer() = default;

    explicit VRViewer(const VRViewer::Configuration & config) : config(config) {};

    void run();

    void finishSetup(void *vkInstance, void *vkPhysicalDevice, void *vkDevice, uint32_t vkQueueFamilyIndex, uint32_t vkQueueIndex);

private:
    Configuration config;
    std::shared_ptr<OXR::OXRContext> context = nullptr;

    void createProjectionLayer();
};


#endif //OPENXRVIEWER_H
