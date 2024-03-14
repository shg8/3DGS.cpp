

#include "ComputePipeline.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context, std::shared_ptr<Shader> shader): Pipeline(context), shader(std::move(shader)) {
    this->shader->load();
}

void ComputePipeline::build() {
    buildPipelineLayout();

    vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eCompute, shader->shader.get(), "main");
    vk::ComputePipelineCreateInfo computePipelineCreateInfo({}, pipelineShaderStageCreateInfo, pipelineLayout.get());
    pipeline = context->device->createComputePipelineUnique(nullptr, computePipelineCreateInfo).value;
}
