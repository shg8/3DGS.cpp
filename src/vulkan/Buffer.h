#ifndef VULKAN_SPLATTING_BUFFER_H
#define VULKAN_SPLATTING_BUFFER_H

#include <cstdint>
#include <memory>

#include "DescriptorSet.h"
#include "VulkanContext.h"
#include "vk_mem_alloc.h"

class DescriptorSet;

class Buffer : public std::enable_shared_from_this<Buffer> {
public:
    Buffer(const std::shared_ptr<VulkanContext>& context, uint32_t size, vk::BufferUsageFlags usage, VmaMemoryUsage vmaUsage,
           VmaAllocationCreateFlags flags, bool concurrentSharing = false, VkDeviceSize alignment = 0, std::string debugName = "Unnamed");

    Buffer(const Buffer &) = delete;

    Buffer(Buffer &&) = delete;

    Buffer &operator=(const Buffer &) = delete;

    Buffer &operator=(Buffer &&) = delete;

    ~Buffer();

    void realloc(uint64_t uint64);

    void boundToDescriptorSet(std::weak_ptr<DescriptorSet> descriptorSet, uint32_t set, uint32_t binding, vk::DescriptorType type);

    static std::shared_ptr<Buffer> uniform(std::shared_ptr<VulkanContext> context, uint32_t size, bool concurrentSharing = false);

    static std::shared_ptr<Buffer> staging(std::shared_ptr<VulkanContext> context, unsigned long size);

    static std::shared_ptr<Buffer> storage(std::shared_ptr<VulkanContext> context, uint64_t size, bool concurrentSharing = false, vk::DeviceSize alignment = 0, std
                                           ::string debugName = "Unnamed Storage Buffer");

    void upload(const void *data, uint32_t size, uint32_t offset = 0);

    void uploadFrom(std::shared_ptr<Buffer> buffer);

    std::vector<char> download();

    void downloadTo(std::shared_ptr<Buffer> buffer, vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0);

    void assertEquals(char *data, size_t length);

    template<typename T>
    T readOne(vk::DeviceSize offset = 0) {
        if (vmaUsage == VMA_MEMORY_USAGE_GPU_ONLY || vmaUsage == VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE) {
            const auto stagingBuffer = Buffer::staging(context, sizeof(T));
            downloadTo(stagingBuffer, offset, 0);
            return *static_cast<T *>(stagingBuffer->allocation_info.pMappedData);
        } else if (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
            return *(static_cast<T *>(allocation_info.pMappedData) + offset / sizeof(T));
        } else {
            throw std::runtime_error("Buffer is not mappable");
        }
    }

    void computeWriteReadBarrier(vk::CommandBuffer commandBuffer);
    void computeReadWriteBarrier(vk::CommandBuffer commandBuffer);
    void computeWriteWriteBarrier(vk::CommandBuffer commandBuffer);

    vk::DeviceSize size;
    vk::BufferUsageFlags usage;
    uint64_t alignment;
    bool shared;

    vk::Buffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;

    VmaMemoryUsage vmaUsage;
    VmaAllocationCreateFlags flags;


private:
    void alloc();

    Buffer createStagingBuffer(uint32_t size);
    std::shared_ptr<VulkanContext> context;

    std::vector<std::tuple<std::weak_ptr<DescriptorSet>, uint32_t, uint32_t, vk::DescriptorType>> boundDescriptorSets;
    std::string debugName;
};


#endif //VULKAN_SPLATTING_BUFFER_H
