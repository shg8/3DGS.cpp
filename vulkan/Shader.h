#ifndef SHADER_H
#define SHADER_H
#include <memory>
#include <string>
#include <utility>

#include "VulkanContext.h"


class Shader {
public:
    Shader(const std::shared_ptr<VulkanContext>& _context, std::string  name)
        : context(_context),
          name(std::move(name)) {
    }

    void load();

    vk::UniqueShaderModule shader;
private:
    const std::string name;
    std::shared_ptr<VulkanContext> context;
};



#endif //SHADER_H
