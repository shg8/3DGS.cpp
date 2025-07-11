#ifndef IMAGESAVER_H
#define IMAGESAVER_H

#include <memory>
#include <string>
#include "vulkan/VulkanContext.h"

class Buffer;

class ImageSaver {
public:
    static void saveImage(const std::shared_ptr<VulkanContext>& context, 
                         const std::shared_ptr<Image>& image, 
                         const std::string& outputPath);

private:
    static void copyImageToBuffer(const std::shared_ptr<VulkanContext>& context,
                                const std::shared_ptr<Image>& image,
                                const std::shared_ptr<Buffer>& buffer);
};

#endif // IMAGESAVER_H