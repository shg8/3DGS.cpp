

#include "ComputePipeline.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext> &context, const std::string &shaderFile) : Pipeline(context), shader(context, shaderFile) {
    shader.load();
}

void ComputePipeline::build() {
    buildPipelineLayout();

    vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eCompute, shader.shader.get(), "main");
    vk::ComputePipelineCreateInfo computePipelineCreateInfo({}, pipelineShaderStageCreateInfo, pipelineLayout.get());
    pipeline = context->device->createComputePipelineUnique(nullptr, computePipelineCreateInfo).value;
}
