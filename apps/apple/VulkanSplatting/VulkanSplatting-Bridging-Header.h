//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

#include "3dgs.h"
#include <memory>
#include <iostream>

static VulkanSplatting *_instance = nullptr;

void vkgs_initialize(VulkanSplatting::RendererConfiguration config) {
    _instance = new VulkanSplatting(config);
//    try {
        _instance->initialize();
//    } catch (const std::exception& e) {
//        // print error
//        std::cerr << e.what() << std::endl;
//    }
}

void vkgs_draw() {
    _instance->draw();
}

void vkgs_pan_translation(float x, float y) {
    _instance->logTranslation(x, y);
}

void vkgs_movement(float x, float y, float z) {
    _instance->logMovement(x, y, z);
}

void vkgs_cleanup() {
    _instance->stop();
    delete _instance;
    _instance = nullptr;
}
