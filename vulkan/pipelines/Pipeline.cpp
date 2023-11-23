#include "Pipeline.h"

#include <utility>

uint32_t Pipeline::DescriptorOption::get(size_t index) const {
    if (multiple) {
        return values[index];
    } else {
        return value;
    }
}

Pipeline::Pipeline(const std::shared_ptr<VulkanContext>& _context) : context(_context) {

}

void Pipeline::addDescriptorSet(uint32_t set, std::shared_ptr<DescriptorSet> descriptorSet) {
    descriptorSets[set] = std::move(descriptorSet);
}

void Pipeline::buildPipelineLayout() {
    std::vector<vk::DescriptorSetLayout> layouts;
    layouts.reserve(descriptorSets.size());
    for (auto &descriptorSet: descriptorSets) {
        layouts.push_back(descriptorSet.second->descriptorSetLayout.get());
    }

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, layouts.size(), layouts.data());
    if (!pushConstantRanges.empty()) {
        pipelineLayoutCreateInfo.setPushConstantRangeCount(pushConstantRanges.size());
        pipelineLayoutCreateInfo.setPPushConstantRanges(pushConstantRanges.data());
    }
    pipelineLayout = context->device->createPipelineLayoutUnique(pipelineLayoutCreateInfo);
}

void Pipeline::bind(const vk::UniqueCommandBuffer &commandBuffer, uint8_t currentFrame, DescriptorOption option) {
    commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, pipeline.get());

    std::vector<vk::DescriptorSet> descriptorSetsToBind;
    descriptorSetsToBind.reserve(descriptorSets.size());
    auto ind = 0;
    for (auto &descriptorSet: descriptorSets) {
        descriptorSetsToBind.push_back(descriptorSet.second->getDescriptorSet(currentFrame, option.get(ind++)));
    }

    commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout.get(), 0, descriptorSetsToBind,
                                      nullptr);
}

void Pipeline::addPushConstant(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size) {
    pushConstantRanges.emplace_back(stageFlags, offset, size);
}
