#ifndef VULKAN_SPLATTING_COMPUTEPIPELINE_H
#define VULKAN_SPLATTING_COMPUTEPIPELINE_H


#include "Pipeline.h"
#include <memory>
#include "../Shader.h"

class ComputePipeline : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext> &context, std::shared_ptr<Shader> shader);;

    void build() override;
private:
    std::shared_ptr<Shader> shader;
};


#endif //VULKAN_SPLATTING_COMPUTEPIPELINE_H
