#include "Shader.h"
#include "Utils.h"

void Shader::load() {
    vk::ShaderModuleCreateInfo create_info;
    if (data == nullptr) {
        auto fn = "shaders/" + filename + ".spv";
        auto shader_code = Utils::readFile(fn);
        if (shader_code.empty()) {
            throw std::runtime_error("Failed to load shader: " + fn);
        }
        create_info.codeSize = shader_code.size();
        create_info.pCode = reinterpret_cast<const uint32_t *>(shader_code.data());
    } else {
        create_info.codeSize = size;
        create_info.pCode = reinterpret_cast<const uint32_t *>(data);
    }
    shader = context->device->createShaderModuleUnique(create_info);
}
