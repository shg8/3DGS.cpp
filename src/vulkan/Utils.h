#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "Buffer.h"

namespace Utils {
    std::vector<char *> stringVectorToCharPtrVector(const std::vector<std::string>& stringVector);
    std::vector<char> readFile(const std::string& path);

    template<uint32_t N>
    inline std::vector<glm::vec<N, float, glm::defaultp>> zipVectors(const std::array<std::vector<float>, N>& vectors) {
        std::vector<glm::vec<N, float, glm::defaultp>> result;
        for (uint32_t i = 0; i < vectors[0].size(); i++) {
            glm::vec<N, float, glm::defaultp> vec;
            for (uint32_t j = 0; j < N; j++) {
                vec[j] = vectors[j][i];
            }
            result.push_back(vec);
        }
        return result;
    }

    template<uint32_t N>
    inline std::vector<float> zipAndFlattenVectors(std::array<std::vector<float>, 48> arrays) {
        std::vector<float> result;
        for (uint32_t i = 0; i < arrays[0].size(); i++) {
            for (uint32_t j = 0; j < N; j++) {
                result.push_back(arrays[j][i]);
            }
        }
        return result;
    }

    class BarrierBuilder {
    public:
        BarrierBuilder& queueFamilyIndex(uint32_t queueFamilyIndex);

        BarrierBuilder& addBufferBarrier(const std::shared_ptr<Buffer>&, vk::AccessFlags srcAccessMask,
                                         vk::AccessFlags dstAccessMask, uint32_t srcQueueFamilyIndex,
                                         uint32_t dstQueueFamilyIndex);

        BarrierBuilder& addBufferBarrier(const std::shared_ptr<Buffer>&, vk::AccessFlags srcAccessMask,
                                         vk::AccessFlags dstAccessMask);

        BarrierBuilder& srcQueueFamilyIndex(uint32_t srcQueueFamilyIndex);

        BarrierBuilder& dstQueueFamilyIndex(uint32_t dstQueueFamilyIndex);

        void build(vk::CommandBuffer commandBuffer, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask) const;
    private:
        std::vector<vk::BufferMemoryBarrier> bufferMemoryBarriers;
        uint32_t _srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };
}

#endif //UTILS_H
