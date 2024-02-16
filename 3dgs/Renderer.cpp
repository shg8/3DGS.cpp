#include "Renderer.h"

#include <fstream>

#include "../vulkan/Swapchain.h"

#include <memory>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../vulkan/Utils.h"

#define SORT_ALLOCATE_MULTIPLIER 10

void Renderer::initialize() {
    initializeVulkan();
    loadSceneToGPU();
    createPreprocessPipeline();
    createPrefixSumPipeline();
    createRadixSortPipeline();
    createPreprocessSortPipeline();
    createTileBoundaryPipeline();
    createRenderPipeline();
    createCommandPool();
    recordPreprocessCommandBuffer();
}

void Renderer::handleInput() {
    auto translation = window->getCursorTranslation();
    auto keys = window->getKeys(); // W, A, S, D

    // rotate camera
    if (translation[0] != 0.0 || translation[1] != 0.0) {
        camera.rotation = glm::rotate(camera.rotation, static_cast<float>(translation[0]) * 0.005f,
                                      glm::vec3(0.0f, -1.0f, 0.0f));
        camera.rotation = glm::rotate(camera.rotation, static_cast<float>(translation[1]) * 0.005f,
                                      glm::vec3(-1.0f, 0.0f, 0.0f));
    }

    // move camera
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
    if (keys[0]) {
        direction += glm::vec3(0.0f, 0.0f, -1.0f);
    }
    if (keys[1]) {
        direction += glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (keys[2]) {
        direction += glm::vec3(0.0f, 0.0f, 1.0f);
    }
    if (keys[3]) {
        direction += glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (direction != glm::vec3(0.0f, 0.0f, 0.0f)) {
        direction = glm::normalize(direction);
        camera.position += (glm::mat4_cast(camera.rotation) * glm::vec4(direction, 1.0f)).xyz() * 0.3f;
    }
}

void Renderer::initializeVulkan() {
    window = std::make_shared<Window>("Vulkan Splatting", 800, 600);
    context = std::make_shared<VulkanContext>(Window::getRequiredInstanceExtensions(), std::vector<std::string>{},
                                              configuration.enableVulkanValidationLayers);

    context->createInstance();
    auto surface = static_cast<vk::SurfaceKHR>(window->createSurface(context));
    context->selectPhysicalDevice(configuration.physicalDeviceId, surface);

    vk::PhysicalDeviceFeatures pdf{};
    vk::PhysicalDeviceVulkan11Features pdf11{};
    vk::PhysicalDeviceVulkan12Features pdf12{};
    pdf.shaderInt64 = true;
    pdf12.shaderFloat16 = true;
    pdf12.shaderBufferInt64Atomics = true;
    pdf12.shaderSharedInt64Atomics = true;

    context->createLogicalDevice(pdf, pdf11, pdf12);
    context->createDescriptorPool(1);

    auto [width, height] = window->getFramebufferSize();
    swapchain = std::make_shared<Swapchain>(context, window, configuration.immediateSwapchain);

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        inflightFences.emplace_back(
            context->device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
    }

    renderFinishedSemaphores.resize(FRAMES_IN_FLIGHT);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        renderFinishedSemaphores[i] = context->device->createSemaphoreUnique(vk::SemaphoreCreateInfo());
    }
}

void Renderer::loadSceneToGPU() {
    scene = std::make_shared<GSScene>(configuration.scene);
    scene->load(context);
    // scene->loadTestScene(context);
}

void Renderer::createPreprocessPipeline() {
    uniformBuffer = Buffer::uniform(context, sizeof(UniformBuffer));
    vertexAttributeBuffer = Buffer::storage(context, scene->getNumVertices() * sizeof(VertexAttributeBuffer), false);
    tileOverlapBuffer = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t), false);

    preprocessPipeline = std::make_shared<ComputePipeline>(context, "preprocess");
    inputSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    inputSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                        scene->vertexBuffer);
    inputSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                        scene->cov3DBuffer);
    inputSet->build();
    preprocessPipeline->addDescriptorSet(0, inputSet);

    auto uniformOutputSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    uniformOutputSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eUniformBuffer,
                                                vk::ShaderStageFlagBits::eCompute,
                                                uniformBuffer);
    uniformOutputSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer,
                                                vk::ShaderStageFlagBits::eCompute,
                                                vertexAttributeBuffer);
    uniformOutputSet->bindBufferToDescriptorSet(2, vk::DescriptorType::eStorageBuffer,
                                                vk::ShaderStageFlagBits::eCompute,
                                                tileOverlapBuffer);
    uniformOutputSet->build();

    preprocessPipeline->addDescriptorSet(1, uniformOutputSet);
    preprocessPipeline->build();
}

Renderer::Renderer(RendererConfiguration configuration) : configuration(std::move(configuration)) {
}

void Renderer::createPrefixSumPipeline() {
    prefixSumPingBuffer = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t), false);
    prefixSumPongBuffer = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t), false);
    totalSumBufferHost = Buffer::staging(context, sizeof(uint32_t));

    prefixSumPipeline = std::make_shared<ComputePipeline>(context, "prefix_sum");
    auto descriptorSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             prefixSumPingBuffer);
    descriptorSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             prefixSumPongBuffer);
    descriptorSet->build();

    prefixSumPipeline->addDescriptorSet(0, descriptorSet);
    prefixSumPipeline->addPushConstant(vk::ShaderStageFlagBits::eCompute, 0, sizeof(uint32_t));
    prefixSumPipeline->build();
}

void Renderer::createRadixSortPipeline() {
    sortKBufferEven = Buffer::storage(context, scene->getNumVertices() * sizeof(uint64_t) * SORT_ALLOCATE_MULTIPLIER,
                                      false);
    sortKBufferOdd = Buffer::storage(context, scene->getNumVertices() * sizeof(uint64_t) * SORT_ALLOCATE_MULTIPLIER,
                                     false);
    sortVBufferEven = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t) * SORT_ALLOCATE_MULTIPLIER,
                                      false);
    sortVBufferOdd = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t) * SORT_ALLOCATE_MULTIPLIER,
                                     false);

    uint32_t globalInvocationSize = scene->getNumVertices() * SORT_ALLOCATE_MULTIPLIER / numRadixSortBlocksPerWorkgroup;
    uint32_t remainder = scene->getNumVertices() * SORT_ALLOCATE_MULTIPLIER % numRadixSortBlocksPerWorkgroup;
    globalInvocationSize += remainder > 0 ? 1 : 0;

    auto numWorkgroups = (globalInvocationSize + 256 - 1) / 256;

    sortHistBuffer = Buffer::storage(context, numWorkgroups * 256 * sizeof(uint32_t), false);

    sortHistPipeline = std::make_shared<ComputePipeline>(context, "hist");
    sortPipeline = std::make_shared<ComputePipeline>(context, "sort");

    auto descriptorSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortKBufferEven);
    descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortKBufferOdd);
    descriptorSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortHistBuffer);
    descriptorSet->build();
    sortHistPipeline->addDescriptorSet(0, descriptorSet);
    sortHistPipeline->addPushConstant(vk::ShaderStageFlagBits::eCompute, 0, sizeof(RadixSortPushConstants));
    sortHistPipeline->build();

    descriptorSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortKBufferEven);
    descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortKBufferOdd);
    descriptorSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortKBufferOdd);
    descriptorSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortKBufferEven);
    descriptorSet->bindBufferToDescriptorSet(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortVBufferEven);
    descriptorSet->bindBufferToDescriptorSet(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortVBufferOdd);
    descriptorSet->bindBufferToDescriptorSet(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortVBufferOdd);
    descriptorSet->bindBufferToDescriptorSet(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortVBufferEven);
    descriptorSet->bindBufferToDescriptorSet(4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortHistBuffer);
    descriptorSet->build();
    sortPipeline->addDescriptorSet(0, descriptorSet);
    sortPipeline->addPushConstant(vk::ShaderStageFlagBits::eCompute, 0, sizeof(RadixSortPushConstants));
    sortPipeline->build();
}

void Renderer::createPreprocessSortPipeline() {
    preprocessSortPipeline = std::make_shared<ComputePipeline>(context, "preprocess_sort");
    auto descriptorSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             vertexAttributeBuffer);
    descriptorSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             prefixSumPingBuffer);
    descriptorSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             prefixSumPongBuffer);
    descriptorSet->bindBufferToDescriptorSet(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortKBufferEven);
    descriptorSet->bindBufferToDescriptorSet(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortVBufferEven);
    descriptorSet->build();

    preprocessSortPipeline->addDescriptorSet(0, descriptorSet);
    preprocessSortPipeline->addPushConstant(vk::ShaderStageFlagBits::eCompute, 0, sizeof(uint32_t));
    preprocessSortPipeline->build();
}

void Renderer::createTileBoundaryPipeline() {
    auto [width, height] = window->getFramebufferSize();
    auto tileX = (width + 16 - 1) / 16;
    auto tileY = (height + 16 - 1) / 16;
    tileBoundaryBuffer = Buffer::storage(context, tileX * tileY * sizeof(uint32_t) * 2, false);

    tileBoundaryPipeline = std::make_shared<ComputePipeline>(context, "tile_boundary");
    auto descriptorSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             sortKBufferEven);
    // descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
    //                                          sortKBufferOdd);
    descriptorSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             tileBoundaryBuffer);
    descriptorSet->build();

    tileBoundaryPipeline->addDescriptorSet(0, descriptorSet);
    tileBoundaryPipeline->addPushConstant(vk::ShaderStageFlagBits::eCompute, 0, sizeof(uint32_t));
    tileBoundaryPipeline->build();
}

void Renderer::createRenderPipeline() {
    renderPipeline = std::make_shared<ComputePipeline>(context, "render");
    auto inputSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    inputSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                        vertexAttributeBuffer);
    inputSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                        tileBoundaryBuffer);
    inputSet->bindBufferToDescriptorSet(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                        sortVBufferEven);
    // inputSet->bindBufferToDescriptorSet(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
    //                                     sortKBufferOdd);
    inputSet->build();

    auto outputSet = std::make_shared<DescriptorSet>(context, 1);
    for (auto&image: swapchain->swapchainImages) {
        outputSet->bindImageToDescriptorSet(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute,
                                            image);
    }
    outputSet->build();
    renderPipeline->addDescriptorSet(0, inputSet);
    renderPipeline->addDescriptorSet(1, outputSet);
    renderPipeline->addPushConstant(vk::ShaderStageFlagBits::eCompute, 0, sizeof(uint32_t) * 2);
    renderPipeline->build();
}

void Renderer::run() {
    auto i = 0;
    while (running) {
        if (!window->tick()) {
            break;
        }
        auto ret = context->device->waitForFences(inflightFences[0].get(), VK_TRUE, UINT64_MAX);
        if (ret != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait for fence");
        }
        context->device->resetFences(inflightFences[0].get());

        auto res = context->device->acquireNextImageKHR(swapchain->swapchain.get(), UINT64_MAX,
                                                        swapchain->imageAvailableSemaphores[0].get(),
                                                        nullptr, &currentImageIndex);
        if (res == vk::Result::eErrorOutOfDateKHR) {
            swapchain->recreate();
            continue;
        }
        else if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("Failed to acquire swapchain image");
        }

        handleInput();

        updateUniforms();

        auto submitInfo = vk::SubmitInfo {}.setCommandBuffers(preprocessCommandBuffer.get());
        context->queues[VulkanContext::Queue::COMPUTE].queue.submit(submitInfo, inflightFences[0].get());

        ret = context->device->waitForFences(inflightFences[0].get(), VK_TRUE, UINT64_MAX);
        if (ret != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait for fence");
        }
        context->device->resetFences(inflightFences[0].get());

        recordRenderCommandBuffer(0);
        vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eComputeShader;
        submitInfo = vk::SubmitInfo {}.setWaitSemaphores(swapchain->imageAvailableSemaphores[0].get())
            .setCommandBuffers(renderCommandBuffer.get())
            .setSignalSemaphores(renderFinishedSemaphores[0].get())
            .setWaitDstStageMask(waitStage);
        context->queues[VulkanContext::Queue::COMPUTE].queue.submit(submitInfo, inflightFences[0].get());

        vk::PresentInfoKHR presentInfo{};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[0].get();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain->swapchain.get();
        presentInfo.pImageIndices = &currentImageIndex;

        ret = context->queues[VulkanContext::Queue::PRESENT].queue.presentKHR(presentInfo);
        if (ret == vk::Result::eErrorOutOfDateKHR || ret == vk::Result::eSuboptimalKHR) {
            swapchain->recreate();
        }
        else if (ret != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to present swapchain image");
        }

        auto now = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFpsTime).count();
        if (diff > 1000) {
            std::cout << "FPS: " << fpsCounter << std::endl;
            fpsCounter = 0;
            lastFpsTime = now;
        } else {
            fpsCounter++;
        }

        // auto nn = totalSumBufferHost->readOne<uint32_t>() ;
        // auto staging = Buffer::staging(context, nn* sizeof(uint64_t));
        // sortKVBufferEven->downloadTo(staging);
        // uint64_t* data = reinterpret_cast<uint64_t*>(staging->allocation_info.pMappedData);
        // for (int i = 0; i < nn; i++) {
        //     auto key = data[i] >> 32;
        //     if (i > 0) {
        //         assert(key >= data[i-1] >> 32);
        //     }
        // }
        //
        // uint32_t totalSum = totalSumBufferHost->readOne<uint32_t>();
        // std::cout << "Total sum: " << totalSum << std::endl;
        // //
        // auto v = tileBoundaryBuffer->download();
        // uint32_t* data2 = reinterpret_cast<uint32_t*>(v.data());
        // for (int i = 0; i < v.size() / sizeof(uint32_t); i+=2) {
        //     assert(data2[i] <= data2[i+1]);
        // }
    }
}

void Renderer::createCommandPool() {
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = context->queues[VulkanContext::Queue::COMPUTE].queueFamily;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    commandPool = context->device->createCommandPoolUnique(poolInfo, nullptr);
}

void Renderer::recordPreprocessCommandBuffer() {
    vk::CommandBufferAllocateInfo allocateInfo = {commandPool.get(), vk::CommandBufferLevel::ePrimary, 1};
    auto buffers = context->device->allocateCommandBuffersUnique(allocateInfo);
    preprocessCommandBuffer = std::move(buffers[0]);

    auto numGroups = (scene->getNumVertices() + 255) / 256;

    preprocessCommandBuffer->begin(vk::CommandBufferBeginInfo{});

    preprocessPipeline->bind(preprocessCommandBuffer, 0, 0);
    preprocessCommandBuffer->dispatch(numGroups, 1, 1);
    tileOverlapBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());

    vk::BufferCopy copyRegion = {0, 0, tileOverlapBuffer->size};
    preprocessCommandBuffer->copyBuffer(tileOverlapBuffer->buffer, prefixSumPingBuffer->buffer, 1, &copyRegion);

    prefixSumPingBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());

    prefixSumPipeline->bind(preprocessCommandBuffer, 0, 0);
    const auto iters = static_cast<uint32_t>(std::ceil(std::log2(static_cast<float>(scene->getNumVertices()))));
    for (uint32_t timestep = 0; timestep <= iters; timestep++) {
        preprocessCommandBuffer->pushConstants(prefixSumPipeline->pipelineLayout.get(),
                                               vk::ShaderStageFlagBits::eCompute, 0,
                                               sizeof(uint32_t), &timestep);
        preprocessCommandBuffer->dispatch(numGroups, 1, 1);

        if (timestep % 2 == 0) {
            prefixSumPongBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());
            prefixSumPingBuffer->computeReadWriteBarrier(preprocessCommandBuffer.get());
        }
        else {
            prefixSumPingBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());
            prefixSumPongBuffer->computeReadWriteBarrier(preprocessCommandBuffer.get());
        }
    }

    auto totalSumRegion = vk::BufferCopy{(scene->getNumVertices() - 1) * sizeof(uint32_t), 0, sizeof(uint32_t)};
    if (iters % 2 == 0) {
        preprocessCommandBuffer->copyBuffer(prefixSumPingBuffer->buffer, totalSumBufferHost->buffer, 1,
                                            &totalSumRegion);
    }
    else {
        preprocessCommandBuffer->copyBuffer(prefixSumPongBuffer->buffer, totalSumBufferHost->buffer, 1,
                                            &totalSumRegion);
    }

    vertexAttributeBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());

    preprocessSortPipeline->bind(preprocessCommandBuffer, 0, iters % 2 == 0 ? 0 : 1);
    uint32_t tileX = (swapchain->swapchainExtent.width + 16 - 1) / 16;
    // assert(tileX == 50);
    preprocessCommandBuffer->pushConstants(preprocessSortPipeline->pipelineLayout.get(),
                                           vk::ShaderStageFlagBits::eCompute, 0,
                                           sizeof(uint32_t), &tileX);
    preprocessCommandBuffer->dispatch(numGroups, 1, 1);

    sortKBufferEven->computeWriteReadBarrier(preprocessCommandBuffer.get());

    preprocessCommandBuffer->end();
}


void Renderer::recordRenderCommandBuffer(uint32_t currentFrame) {
    if (!renderCommandBuffer) {
        renderCommandBuffer = std::move(context->device->allocateCommandBuffersUnique(
            vk::CommandBufferAllocateInfo(commandPool.get(), vk::CommandBufferLevel::ePrimary, 1))[0]);
    }
    renderCommandBuffer->reset({});
    renderCommandBuffer->begin(vk::CommandBufferBeginInfo{});

    uint32_t numInstances = totalSumBufferHost->readOne<uint32_t>();
    // std::cout << "Num instances: " << numInstances << std::endl;
    if (numInstances > scene->getNumVertices() * SORT_ALLOCATE_MULTIPLIER) {
        throw std::runtime_error("Gaussian instantiation out of memory");
    }
    assert(numInstances <= scene->getNumVertices() * SORT_ALLOCATE_MULTIPLIER);
    for (auto i = 0; i < 8; i++) {
        sortHistPipeline->bind(renderCommandBuffer, 0, i % 2 == 0 ? 0 : 1);
        auto invocationSize = (numInstances + numRadixSortBlocksPerWorkgroup - 1) / numRadixSortBlocksPerWorkgroup;
        invocationSize = (invocationSize + 255) / 256;

        RadixSortPushConstants pushConstants{};
        pushConstants.g_num_elements = numInstances;
        pushConstants.g_num_blocks_per_workgroup = numRadixSortBlocksPerWorkgroup;
        pushConstants.g_shift = i * 8;
        pushConstants.g_num_workgroups = invocationSize;
        renderCommandBuffer->pushConstants(sortHistPipeline->pipelineLayout.get(),
                                           vk::ShaderStageFlagBits::eCompute, 0,
                                           sizeof(RadixSortPushConstants), &pushConstants);

        renderCommandBuffer->dispatch(invocationSize, 1, 1);

        sortHistBuffer->computeWriteReadBarrier(renderCommandBuffer.get());

        sortPipeline->bind(renderCommandBuffer, 0, i % 2 == 0 ? 0 : 1);
        renderCommandBuffer->pushConstants(sortPipeline->pipelineLayout.get(),
                                           vk::ShaderStageFlagBits::eCompute, 0,
                                           sizeof(RadixSortPushConstants), &pushConstants);
        renderCommandBuffer->dispatch(invocationSize, 1, 1);

        if (i % 2 == 0) {
            sortKBufferOdd->computeWriteReadBarrier(renderCommandBuffer.get());
            sortVBufferOdd->computeWriteReadBarrier(renderCommandBuffer.get());
        }
        else {
            sortKBufferEven->computeWriteReadBarrier(renderCommandBuffer.get());
            sortVBufferEven->computeWriteReadBarrier(renderCommandBuffer.get());
        }
    }

    renderCommandBuffer->fillBuffer(tileBoundaryBuffer->buffer, 0, VK_WHOLE_SIZE, 0);

    Utils::BarrierBuilder().queueFamilyIndex(context->queues[VulkanContext::Queue::COMPUTE].queueFamily)
            .addBufferBarrier(tileBoundaryBuffer, vk::AccessFlagBits::eTransferWrite,
                              vk::AccessFlagBits::eShaderWrite)
            .build(renderCommandBuffer.get(), vk::PipelineStageFlagBits::eTransfer,
                   vk::PipelineStageFlagBits::eComputeShader);

    // Since we have 64 bit keys, the sort result is always in the even buffer
    tileBoundaryPipeline->bind(renderCommandBuffer, 0, 0);
    renderCommandBuffer->pushConstants(tileBoundaryPipeline->pipelineLayout.get(),
                                       vk::ShaderStageFlagBits::eCompute, 0,
                                       sizeof(uint32_t), &numInstances);
    renderCommandBuffer->dispatch((numInstances + 255) / 256, 1, 1);

    tileBoundaryBuffer->computeWriteReadBarrier(renderCommandBuffer.get());

    renderPipeline->bind(renderCommandBuffer, 0, std::vector<uint32_t>{0, currentImageIndex});
    auto [width, height] = window->getFramebufferSize();
    uint32_t constants[2] = {width, height};
    renderCommandBuffer->pushConstants(renderPipeline->pipelineLayout.get(),
                                       vk::ShaderStageFlagBits::eCompute, 0,
                                       sizeof(uint32_t) * 2, constants);

    // image layout transition: undefined -> general
    vk::ImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
    imageMemoryBarrier.newLayout = vk::ImageLayout::eGeneral;
    imageMemoryBarrier.image = swapchain->swapchainImages[currentImageIndex]->image;
    imageMemoryBarrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    renderCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                         vk::PipelineStageFlagBits::eComputeShader,
                                         vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);

    renderCommandBuffer->dispatch((width + 15) / 16, (height + 15) / 16, 1);

    // image layout transition: general -> present
    imageMemoryBarrier.oldLayout = vk::ImageLayout::eGeneral;
    imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    renderCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                                         vk::PipelineStageFlagBits::eBottomOfPipe,
                                         vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
    renderCommandBuffer->end();
}

void Renderer::updateUniforms() {
    UniformBuffer data{};
    auto [width, height] = window->getFramebufferSize();
    data.width = width;
    data.height = height;
    data.camera_position = glm::vec4(camera.position, 1.0f);

    auto rotation = glm::mat4_cast(camera.rotation);
    auto translation = glm::translate(glm::mat4(1.0f), camera.position);
    auto view = glm::inverse(translation * rotation);

    data.view_mat = view;
    data.proj_mat = glm::perspective(glm::radians(camera.fov), static_cast<float>(width) / static_cast<float>(height),
                                     camera.nearPlane,
                                     camera.farPlane) * view;

    data.view_mat[0][1] *= -1.0f;
    data.view_mat[1][1] *= -1.0f;
    data.view_mat[2][1] *= -1.0f;
    data.view_mat[3][1] *= -1.0f;
    data.view_mat[0][2] *= -1.0f;
    data.view_mat[1][2] *= -1.0f;
    data.view_mat[2][2] *= -1.0f;
    data.view_mat[3][2] *= -1.0f;

    data.proj_mat[0][1] *= -1.0f;
    data.proj_mat[1][1] *= -1.0f;
    data.proj_mat[2][1] *= -1.0f;
    data.proj_mat[3][1] *= -1.0f;
    data.tan_fovx = std::tan(glm::radians(camera.fov) / 2.0);
    data.tan_fovy = data.tan_fovx * static_cast<float>(height) / static_cast<float>(width);
    uniformBuffer->upload(&data, sizeof(UniformBuffer), 0);
}

Renderer::~Renderer() {

}
