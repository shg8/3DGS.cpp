#ifndef OPENXRVIEWER_H
#define OPENXRVIEWER_H
#include <args.hxx>
#include <barrier>
#include <memory>
#include <utility>

#include "OXRContext.h"

class VRViewer {
public:
    struct Configuration {
        std::string scenePath;
    };

    VRViewer() = default;

    explicit VRViewer(const VRViewer::Configuration & config) : config(config) {};

    VRViewer(const VRViewer &other) = delete;

    VRViewer(VRViewer &&other) = delete;

    VRViewer & operator=(const VRViewer &other) = delete;

    VRViewer & operator=(VRViewer &&other) = delete;

    void run();

    void finishSetup(void *vkInstance, void *vkPhysicalDevice, void *vkDevice, uint32_t vkQueueFamilyIndex, uint32_t vkQueueIndex);

private:
    Configuration config;
    std::shared_ptr<OXRContext> context = nullptr;
};


#endif //OPENXRVIEWER_H
