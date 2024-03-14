#include "DescriptorSet.h"

void DescriptorSet::bindBufferToDescriptorSet(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlagBits stage,
                                              std::shared_ptr<Buffer> buffer) {
    const vk::DescriptorSetLayoutBinding layoutBinding{binding, type, 1, stage};

    const auto&bindingVector = bindings[binding];
    if (!bindingVector.empty() && bindingVector[bindingVector.size() - 1].layoutBinding != layoutBinding) {
        throw std::runtime_error(
            "Binding " + std::to_string(binding) + " already exists with different layout binding");
    }

    bindings[binding].push_back(DescriptorBinding{
        type, layoutBinding, buffer,
        vk::DescriptorBufferInfo(buffer->buffer, 0, buffer->size)
    });
}

void DescriptorSet::build() {
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    for (auto&[_, options]: bindings) {
        layoutBindings.push_back(options[0].layoutBinding);
        if (options.size() != 1) {
            if (maxOptions == 1) {
                maxOptions = options.size();
            }
            else if (maxOptions != options.size()) {
                throw std::runtime_error(
                    "Inconsistent number of alternative buffers " + std::to_string(options[0].layoutBinding.binding));
            }
        }
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo({}, layoutBindings.size(), layoutBindings.data());
    descriptorSetLayout = context->device->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);

    std::vector<vk::DescriptorSetLayout> layouts(framesInFlight * maxOptions, descriptorSetLayout.get());
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(context->descriptorPool.get(), layouts.size(),
                                                            layouts.data());
    descriptorSets = context->device->allocateDescriptorSetsUnique(descriptorSetAllocateInfo);

    for (int i = 0; i < framesInFlight; i++) {
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        for (auto&binding: bindings) {
            for (auto j = 0; j < maxOptions; j++) {
                if (binding.second.size() == 1) {
                    if (binding.second[0].buffer != nullptr) {
                        binding.second[0].buffer->boundToDescriptorSet(static_cast<std::weak_ptr<DescriptorSet>>(shared_from_this()), i * maxOptions + j, binding.first, binding.second[0].type);
                        writeDescriptorSets.emplace_back(descriptorSets[i * maxOptions + j].get(), binding.first, 0, 1,
                                                     binding.second[0].type, nullptr,
                                                     &binding.second[0].bufferInfo);
                    } else {
                        writeDescriptorSets.emplace_back(descriptorSets[i * maxOptions + j].get(), binding.first, 0, 1,
                                                         binding.second[0].type, &binding.second[0].imageInfo,
                                                         nullptr);
                    }
                }
                else {
                    if (binding.second.at(j).buffer != nullptr) {
                        binding.second.at(j).buffer->boundToDescriptorSet(static_cast<std::weak_ptr<DescriptorSet>>(shared_from_this()), i * maxOptions + j, binding.first, binding.second.at(j).type);
                        writeDescriptorSets.emplace_back(descriptorSets[i * maxOptions + j].get(), binding.first, 0, 1,
                                                         binding.second.at(j).type, nullptr,
                                                         &binding.second.at(j).bufferInfo);
                    } else {
                        writeDescriptorSets.emplace_back(descriptorSets[i * maxOptions + j].get(), binding.first, 0, 1,
                                                         binding.second.at(j).type, &binding.second.at(j).imageInfo,
                                                         nullptr);
                    }
                }
            }
        }
        context->device->updateDescriptorSets(writeDescriptorSets, nullptr);
    }
}

vk::DescriptorSet DescriptorSet::getDescriptorSet(uint8_t currentFrame, uint8_t option) const {
    if (option >= maxOptions) {
        throw std::runtime_error("Invalid option " + std::to_string(option));
    }
    return descriptorSets[currentFrame * maxOptions + option].get();
}

void DescriptorSet::bindImageToDescriptorSet(uint32_t i, vk::DescriptorType descriptor, vk::ShaderStageFlagBits stage,
                                             std::shared_ptr<Image> image) {
    const vk::DescriptorSetLayoutBinding layoutBinding{i, descriptor, 1, stage};
    const auto&bindingVector = bindings[i];
    if (!bindingVector.empty() && bindingVector[bindingVector.size() - 1].layoutBinding != layoutBinding) {
        throw std::runtime_error(
            "Binding " + std::to_string(i) + " already exists with different layout binding");
    }

    bindings[i].push_back(DescriptorBinding{
        descriptor, layoutBinding, nullptr, {},
        image,
        vk::DescriptorImageInfo({}, image->imageView.get(), vk::ImageLayout::eGeneral)
    });
}

DescriptorSet::DescriptorSet(const std::shared_ptr<VulkanContext>&_context, uint8_t framesInFlight) : context(_context),
    framesInFlight(framesInFlight) {
}
