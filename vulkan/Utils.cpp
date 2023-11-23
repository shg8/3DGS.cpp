#include <fstream>
#include <utility>
#include "Utils.h"

#include "Buffer.h"

std::vector<char *> Utils::stringVectorToCharPtrVector(const std::vector<std::string>&stringVector) {
    std::vector<char *> charPtrVector;
    for (auto&string: stringVector) {
        charPtrVector.push_back(const_cast<char *>(string.c_str()));
    }
    return charPtrVector;
}

std::vector<char> Utils::readFile(const std::string&path) {
    std::vector<char> result;
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return result;
    }
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    result.resize(fileSize);
    file.read(result.data(), fileSize);
    if (!file) {
        result.clear();
    }
    file.close();
    return result;
}

Utils::BarrierBuilder& Utils::BarrierBuilder::queueFamilyIndex(uint32_t queueFamilyIndex) {
    this->_srcQueueFamilyIndex = queueFamilyIndex;
    this->_dstQueueFamilyIndex = queueFamilyIndex;
    return *this;
}

Utils::BarrierBuilder& Utils::BarrierBuilder::addBufferBarrier(const std::shared_ptr<Buffer>&buffer,
                                                               const vk::AccessFlags srcAccessMask,
                                                               const vk::AccessFlags dstAccessMask,
                                                               const uint32_t srcQueueFamilyIndex,
                                                               const uint32_t dstQueueFamilyIndex) {
    bufferMemoryBarriers.emplace_back(srcAccessMask, dstAccessMask, srcQueueFamilyIndex,
                                                              dstQueueFamilyIndex, buffer->buffer, 0,
                                                              buffer->size);
    return *this;
}

Utils::BarrierBuilder& Utils::BarrierBuilder::addBufferBarrier(const std::shared_ptr<Buffer>&buffer,
                                                               const vk::AccessFlags srcAccessMask,
                                                               const vk::AccessFlags dstAccessMask) {
    return addBufferBarrier(buffer, srcAccessMask, dstAccessMask, _srcQueueFamilyIndex,
                            _dstQueueFamilyIndex);
}

Utils::BarrierBuilder& Utils::BarrierBuilder::srcQueueFamilyIndex(uint32_t srcQueueFamilyIndex) {
    this->_srcQueueFamilyIndex = srcQueueFamilyIndex;
    return *this;
}

Utils::BarrierBuilder& Utils::BarrierBuilder::dstQueueFamilyIndex(uint32_t dstQueueFamilyIndex) {
    this->_dstQueueFamilyIndex = dstQueueFamilyIndex;
    return *this;
}

void Utils::BarrierBuilder::build(const vk::CommandBuffer commandBuffer, const vk::PipelineStageFlags srcStageMask,
                                  const vk::PipelineStageFlags dstStageMask) const {
    commandBuffer.pipelineBarrier(srcStageMask, dstStageMask, vk::DependencyFlags(), 0, nullptr,
                                  bufferMemoryBarriers.size(), bufferMemoryBarriers.data(), 0, nullptr);
}
