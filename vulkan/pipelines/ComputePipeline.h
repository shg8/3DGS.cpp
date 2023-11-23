

#ifndef VULKAN_SPLATTING_COMPUTEPIPELINE_H
#define VULKAN_SPLATTING_COMPUTEPIPELINE_H


#include "Pipeline.h"

class ComputePipeline : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext> &context, const std::string& shader);

    void build() override;
private:
    Shader shader;
};


#endif //VULKAN_SPLATTING_COMPUTEPIPELINE_H
