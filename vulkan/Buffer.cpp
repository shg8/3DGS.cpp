#include <iostream>
#include <utility>
#include "Buffer.h"

#include "Utils.h"

Buffer::Buffer(const std::shared_ptr<VulkanContext>& _context, uint32_t size, vk::BufferUsageFlags usage,
               VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags flags, bool shared, vk::DeviceSize alignment)
        : context(_context),
          size(size),
          usage(usage),
          vmaUsage(vmaUsage),
          flags(flags),
          allocation(nullptr) {
    auto bufferInfo = vk::BufferCreateInfo()
            .setSize(size)
            .setUsage(usage)
            .setSharingMode(shared ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive);
    if (shared) {
        auto graphicsFamily = context->queues[VulkanContext::Queue::Type::GRAPHICS].queueFamily;
        auto computeFamily = context->queues[VulkanContext::Queue::Type::COMPUTE].queueFamily;
        uint32_t queueFamilyIndices[] = {graphicsFamily, computeFamily};
        bufferInfo.setQueueFamilyIndexCount(2).setPQueueFamilyIndices(queueFamilyIndices);
    }

    auto vkBufferInfo = static_cast<VkBufferCreateInfo>(bufferInfo);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = vmaUsage;
    allocInfo.flags = flags;

    VkBuffer vkBuffer;

    VkResult res;
    if (alignment != 0) {
        res = vmaCreateBufferWithAlignment(context->allocator, &vkBufferInfo, &allocInfo, alignment, &vkBuffer, &allocation, &allocation_info);
    } else {
        res = vmaCreateBuffer(context->allocator, &vkBufferInfo, &allocInfo, &vkBuffer, &allocation, &allocation_info);
    }
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer");
    }
    buffer = static_cast<vk::Buffer>(vkBuffer);
}

Buffer Buffer::createStagingBuffer(uint32_t size) {
    return Buffer(context, size, vk::BufferUsageFlagBits::eTransferSrc,
                  VMA_MEMORY_USAGE_AUTO,
                  VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, false);
}

void Buffer::upload(const void *data, uint32_t size, uint32_t offset) {
    if (size + offset > this->size) {
        throw std::runtime_error("Buffer overflow");
    }

    if (vmaUsage == VMA_MEMORY_USAGE_GPU_ONLY || vmaUsage == VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE) {
        auto stagingBuffer = createStagingBuffer(size);
        memcpy(stagingBuffer.allocation_info.pMappedData, ((char*) data) + offset, size);
        auto commandBuffer = context->beginOneTimeCommandBuffer();
        vk::BufferCopy copyRegion = {};
        copyRegion.setSize(size);
        commandBuffer->copyBuffer(stagingBuffer.buffer, buffer, 1, &copyRegion);
        context->endOneTimeCommandBuffer(std::move(commandBuffer), VulkanContext::Queue::COMPUTE);
    } else if (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
        memcpy(allocation_info.pMappedData, ((char *) data) + offset, size);
    } else {
        throw std::runtime_error("Buffer is not mappable");
    }
}

void Buffer::uploadFrom(std::shared_ptr<Buffer> buffer) {
    if (buffer->size > size) {
        throw std::runtime_error("Buffer overflow");
    }

    if (vmaUsage == VMA_MEMORY_USAGE_GPU_ONLY || vmaUsage == VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE) {
        auto commandBuffer = context->beginOneTimeCommandBuffer();
        vk::BufferCopy copyRegion = {};
        copyRegion.setSize(buffer->size);
        commandBuffer->copyBuffer(buffer->buffer, this->buffer, 1, &copyRegion);
        context->endOneTimeCommandBuffer(std::move(commandBuffer), VulkanContext::Queue::COMPUTE);
    } else if (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
        memcpy(allocation_info.pMappedData, buffer->allocation_info.pMappedData, buffer->size);
    } else {
        throw std::runtime_error("Buffer is not mappable");
    }
}

void Buffer::downloadTo(std::shared_ptr<Buffer> buffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset) {
    if (vmaUsage == VMA_MEMORY_USAGE_GPU_ONLY || vmaUsage == VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE) {
        auto commandBuffer = context->beginOneTimeCommandBuffer();
        vk::BufferCopy copyRegion = {};
        copyRegion.setSize(buffer->size);
        commandBuffer->copyBuffer(this->buffer, buffer->buffer, 1, &copyRegion);
        context->endOneTimeCommandBuffer(std::move(commandBuffer), VulkanContext::Queue::COMPUTE);
    } else if (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
        memcpy(buffer->allocation_info.pMappedData, allocation_info.pMappedData, buffer->size);
    } else {
        throw std::runtime_error("Buffer is not mappable");
    }
}

Buffer::~Buffer() {
    vmaDestroyBuffer(context->allocator, buffer, allocation);
    std::cout << "Destroyed buffer" << std::endl;
}

std::shared_ptr<Buffer> Buffer::uniform(std::shared_ptr<VulkanContext> context, uint32_t size, bool concurrentSharing) {
    return std::make_shared<Buffer>(std::move(context), size, vk::BufferUsageFlagBits::eUniformBuffer,
                  VMA_MEMORY_USAGE_AUTO,
                  VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, concurrentSharing);
}

std::shared_ptr<Buffer> Buffer::staging(std::shared_ptr<VulkanContext> context, unsigned long size) {
    return std::make_shared<Buffer>(context, size, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
                                    VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                           VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                                    false);
}

std::shared_ptr<Buffer> Buffer::storage(std::shared_ptr<VulkanContext> context, uint64_t size, bool concurrentSharing, vk::DeviceSize alignment) {
    return std::make_shared<Buffer>(context, size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc,
                                              VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                              concurrentSharing, alignment);
}

void Buffer::assertEquals(char *data, size_t length) {
    if (length > size) {
        throw std::runtime_error("Buffer overflow");
    }

    if (vmaUsage == VMA_MEMORY_USAGE_GPU_ONLY || vmaUsage == VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE) {
        auto stagingBuffer = Buffer::staging(context, length);
        downloadTo(stagingBuffer);
        if (memcmp(data, stagingBuffer->allocation_info.pMappedData, length) != 0) {
            throw std::runtime_error("Buffer content does not match");
        }
    } else if (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
        if (memcmp(data, allocation_info.pMappedData, length) != 0) {
            throw std::runtime_error("Buffer content does not match");
        }
    } else {
        throw std::runtime_error("Buffer is not mappable");
    }
}

void Buffer::computeWriteReadBarrier(vk::CommandBuffer commandBuffer) {
    Utils::BarrierBuilder().queueFamilyIndex(context->queues[VulkanContext::Queue::COMPUTE].queueFamily)
            .addBufferBarrier(shared_from_this(), vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead)
            .build(commandBuffer, vk::PipelineStageFlagBits::eComputeShader,
                   vk::PipelineStageFlagBits::eComputeShader);
}

void Buffer::computeReadWriteBarrier(vk::CommandBuffer commandBuffer) {
    Utils::BarrierBuilder().queueFamilyIndex(context->queues[VulkanContext::Queue::COMPUTE].queueFamily)
            .addBufferBarrier(shared_from_this(), vk::AccessFlagBits::eShaderRead,
                              vk::AccessFlagBits::eShaderWrite)
            .build(commandBuffer, vk::PipelineStageFlagBits::eComputeShader,
                   vk::PipelineStageFlagBits::eComputeShader);
}

void Buffer::computeWriteWriteBarrier(vk::CommandBuffer commandBuffer) {
    Utils::BarrierBuilder().queueFamilyIndex(context->queues[VulkanContext::Queue::COMPUTE].queueFamily)
            .addBufferBarrier(shared_from_this(), vk::AccessFlagBits::eShaderWrite,
                              vk::AccessFlagBits::eShaderWrite)
            .build(commandBuffer, vk::PipelineStageFlagBits::eComputeShader,
                   vk::PipelineStageFlagBits::eComputeShader);
}

std::vector<char> Buffer::download() {
    auto stagingBuffer = Buffer::staging(context, size);
    downloadTo(stagingBuffer);
    return {(char *) stagingBuffer->allocation_info.pMappedData,
                             ((char *) stagingBuffer->allocation_info.pMappedData) + size};
}
