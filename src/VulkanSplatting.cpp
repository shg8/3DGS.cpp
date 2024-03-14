#include "VulkanSplatting.h"
#include "Renderer.h"

void VulkanSplatting::start() const {
    // Create the renderer
    Renderer renderer(configuration);
    renderer.initialize();
    renderer.createGui();
    renderer.run();
}
