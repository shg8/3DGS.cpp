#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "ImageSaver.h"
#include "vulkan/Buffer.h"
#include <spdlog/spdlog.h>

void ImageSaver::saveImage(const std::shared_ptr<VulkanContext>& context, 
                          const std::shared_ptr<Image>& image, 
                          const std::string& outputPath) {
    
    spdlog::info("Starting image save to {} ({}x{})", outputPath, image->extent.width, image->extent.height);
    
    auto imageSize = image->extent.width * image->extent.height * 4; // RGBA
    
    auto buffer = std::make_shared<Buffer>(context, imageSize, 
                                          vk::BufferUsageFlagBits::eTransferDst,
                                          VMA_MEMORY_USAGE_CPU_ONLY, 
                                          VMA_ALLOCATION_CREATE_MAPPED_BIT);

    spdlog::debug("Created transfer buffer of size {}", imageSize);
    
    copyImageToBuffer(context, image, buffer);
    
    spdlog::debug("Image copied to buffer");

    auto* data = static_cast<uint8_t*>(buffer->allocation_info.pMappedData);
    
    if (!data) {
        throw std::runtime_error("Buffer data is null - memory mapping failed");
    }
    
    spdlog::debug("Starting format conversion for {}x{} image", image->extent.width, image->extent.height);
    
    // Convert BGRA to RGBA if needed
    for (uint32_t i = 0; i < image->extent.width * image->extent.height; ++i) {
        uint8_t* pixel = &data[i * 4];
        std::swap(pixel[0], pixel[2]); // Swap B and R channels
    }

    spdlog::debug("Format conversion complete, writing PNG file");

    int result = stbi_write_png(outputPath.c_str(), 
                               static_cast<int>(image->extent.width), 
                               static_cast<int>(image->extent.height), 
                               4, data, 
                               static_cast<int>(image->extent.width * 4));

    if (result == 0) {
        throw std::runtime_error("Failed to save image to " + outputPath);
    }

    spdlog::info("Saved image to {}", outputPath);
}

void ImageSaver::copyImageToBuffer(const std::shared_ptr<VulkanContext>& context,
                                  const std::shared_ptr<Image>& image,
                                  const std::shared_ptr<Buffer>& buffer) {
    
    auto commandPool = context->device->createCommandPoolUnique(
        vk::CommandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eTransient, 
                                 context->queues[VulkanContext::Queue::GRAPHICS].queueFamily});

    auto commandBuffer = std::move(context->device->allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo(commandPool.get(), vk::CommandBufferLevel::ePrimary, 1))[0]);

    commandBuffer->begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    // Transition image layout for transfer
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;  // Swapchain images are in present layout
    barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image->image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

    commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                  vk::PipelineStageFlagBits::eTransfer,
                                  {}, {}, {}, barrier);

    // Copy image to buffer
    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{image->extent.width, image->extent.height, 1};

    commandBuffer->copyImageToBuffer(image->image, vk::ImageLayout::eTransferSrcOptimal, 
                                    buffer->buffer, region);

    commandBuffer->end();

    auto submitInfo = vk::SubmitInfo{}.setCommandBuffers(commandBuffer.get());
    context->queues[VulkanContext::Queue::GRAPHICS].queue.submit(submitInfo);
    context->queues[VulkanContext::Queue::GRAPHICS].queue.waitIdle();
}