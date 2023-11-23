#include "Shader.h"
#include "Utils.h"

void Shader::load() {
    auto filename = "shaders/" + name + ".comp.spv";
    auto shader_code = Utils::readFile(filename);
    if (shader_code.empty()) {
        throw std::runtime_error("Failed to load shader: " + filename);
    }
    vk::ShaderModuleCreateInfo create_info;
    create_info.codeSize = shader_code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(shader_code.data());
    shader = context->device->createShaderModuleUnique(create_info);
}
