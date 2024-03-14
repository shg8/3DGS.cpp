#ifndef VULKAN_SPLATTING_DESCRIPTORSET_H
#define VULKAN_SPLATTING_DESCRIPTORSET_H


#include <memory>
#include "VulkanContext.h"
#include "Buffer.h"
#include <unordered_map>

#include "Swapchain.h"

class Buffer;

class DescriptorSet : public std::enable_shared_from_this<DescriptorSet> {
public:
    struct DescriptorBinding {
        vk::DescriptorType type;
        vk::DescriptorSetLayoutBinding layoutBinding;

        // buffer info
        std::shared_ptr<Buffer> buffer;
        vk::DescriptorBufferInfo bufferInfo;

        // image info
        std::shared_ptr<Image> image;
        vk::DescriptorImageInfo imageInfo;
    };

    explicit DescriptorSet(const std::shared_ptr<VulkanContext> &context, uint8_t framesInFlight = 1);

    void bindBufferToDescriptorSet(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlagBits stage, std::shared_ptr<Buffer> buffer);

    void build();

    vk::DescriptorSet getDescriptorSet(uint8_t currentFrame, uint8_t option) const;

    void bindImageToDescriptorSet(uint32_t i, vk::DescriptorType descriptor, vk::ShaderStageFlagBits stage, std::shared_ptr<Image> image);

    vk::UniqueDescriptorSetLayout descriptorSetLayout;

    std::vector<vk::UniqueDescriptorSet> descriptorSets;
    size_t maxOptions = 1;

private:
    const std::shared_ptr<VulkanContext> context;
    const uint8_t framesInFlight;
    std::unordered_map<uint32_t, std::vector<DescriptorBinding>> bindings;
};


#endif //VULKAN_SPLATTING_DESCRIPTORSET_H
