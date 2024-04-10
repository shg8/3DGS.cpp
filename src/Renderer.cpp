#include "Renderer.h"

#include <fstream>

#include "vulkan/Swapchain.h"

#include <memory>
#include "shaders.h"
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "vulkan/Utils.h"

#include <spdlog/spdlog.h>

void Renderer::initialize() {
    initializeVulkan();
    createGui();
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

    if ((!configuration.enableGui || (!guiManager.wantCaptureMouse() && !guiManager.mouseCapture)) && window->
        getMouseButton()[0]) {
        window->mouseCapture(true);
        guiManager.mouseCapture = true;
    }

    // rotate camera
    if (!configuration.enableGui || guiManager.mouseCapture) {
        if (translation[0] != 0.0 || translation[1] != 0.0) {
            camera.rotation = glm::rotate(camera.rotation, static_cast<float>(translation[0]) * 0.005f,
                                          glm::vec3(0.0f, -1.0f, 0.0f));
            camera.rotation = glm::rotate(camera.rotation, static_cast<float>(translation[1]) * 0.005f,
                                          glm::vec3(-1.0f, 0.0f, 0.0f));
        }
    }

    // move camera
    if (!configuration.enableGui || !guiManager.wantCaptureKeyboard()) {
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
        if (keys[4]) {
            direction += glm::vec3(0.0f, 1.0f, 0.0f);
        }
        if (keys[5]) {
            direction += glm::vec3(0.0f, -1.0f, 0.0f);
        }
        if (keys[6]) {
            window->mouseCapture(false);
            guiManager.mouseCapture = false;
        }
        if (direction != glm::vec3(0.0f, 0.0f, 0.0f)) {
            direction = glm::normalize(direction);
            camera.position += (glm::mat4_cast(camera.rotation) * glm::vec4(direction, 1.0f)).xyz() * 0.3f;
        }
    }
}

void Renderer::retrieveTimestamps() {
    std::vector<uint64_t> timestamps(queryManager->nextId);
    auto res = context->device->getQueryPoolResults(context->queryPool.get(), 0, queryManager->nextId,
                                                    timestamps.size() * sizeof(uint64_t),
                                                    timestamps.data(), sizeof(uint64_t),
                                                    vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);
    if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to retrieve timestamps");
    }

    auto metrics = queryManager->parseResults(timestamps);
    for (auto& metric: metrics) {
        if (configuration.enableGui)
            guiManager.pushMetric(metric.first, metric.second / 1000000.0);
    }
}

void Renderer::recreateSwapchain() {
    auto oldExtent = swapchain->swapchainExtent;
    spdlog::debug("Recreating swapchain");
    swapchain->recreate();
    if (swapchain->swapchainExtent == oldExtent) {
        return;
    }

    auto [width, height] = swapchain->swapchainExtent;
    auto tileX = (width + 16 - 1) / 16;
    auto tileY = (height + 16 - 1) / 16;
    tileBoundaryBuffer->realloc(tileX * tileY * sizeof(uint32_t) * 2);

    recordPreprocessCommandBuffer();
    createRenderPipeline();
}

void Renderer::initializeVulkan() {
    spdlog::debug("Initializing Vulkan");
    window = configuration.window;
    context = std::make_shared<VulkanContext>(window->getRequiredInstanceExtensions(), std::vector<std::string>{},
                                              configuration.enableVulkanValidationLayers);

    context->createInstance();
    auto surface = static_cast<vk::SurfaceKHR>(window->createSurface(context));
    context->selectPhysicalDevice(configuration.physicalDeviceId, surface);

    vk::PhysicalDeviceFeatures pdf{};
    vk::PhysicalDeviceVulkan11Features pdf11{};
    vk::PhysicalDeviceVulkan12Features pdf12{};
    pdf.shaderStorageImageWriteWithoutFormat = true;
    pdf.shaderInt64 = true;
    // pdf.robustBufferAccess = true;
    // pdf12.shaderFloat16 = true;]
#ifndef __APPLE__
    pdf12.shaderBufferInt64Atomics = true;
    pdf12.shaderSharedInt64Atomics = true;
#endif

    context->createLogicalDevice(pdf, pdf11, pdf12);
    context->createDescriptorPool(1);

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
    spdlog::debug("Loading scene to GPU");
    scene = std::make_shared<GSScene>(configuration.scene);
    scene->load(context);

    // reset descriptor pool
    context->device->resetDescriptorPool(context->descriptorPool.get());
}

void Renderer::createPreprocessPipeline() {
    spdlog::debug("Creating preprocess pipeline");
    uniformBuffer = Buffer::uniform(context, sizeof(UniformBuffer));
    vertexAttributeBuffer = Buffer::storage(context, scene->getNumVertices() * sizeof(VertexAttributeBuffer), false);
    tileOverlapBuffer = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t), false);

    preprocessPipeline = std::make_shared<ComputePipeline>(
        context, std::make_shared<Shader>(context, "preprocess", SPV_PREPROCESS, SPV_PREPROCESS_len));
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

Renderer::Renderer(VulkanSplatting::RendererConfiguration configuration) : configuration(std::move(configuration)) {
}

void Renderer::createGui() {
    if (!configuration.enableGui) {
        return;
    }

    spdlog::debug("Creating GUI");

    imguiManager = std::make_shared<ImguiManager>(context, swapchain, window);
    imguiManager->init();
    guiManager.init();
}

void Renderer::createPrefixSumPipeline() {
    spdlog::debug("Creating prefix sum pipeline");
    prefixSumPingBuffer = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t), false);
    prefixSumPongBuffer = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t), false);
    totalSumBufferHost = Buffer::staging(context, sizeof(uint32_t));

    prefixSumPipeline = std::make_shared<ComputePipeline>(
        context, std::make_shared<Shader>(context, "prefix_sum", SPV_PREFIX_SUM, SPV_PREFIX_SUM_len));
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
    spdlog::debug("Creating radix sort pipeline");
    sortKBufferEven = Buffer::storage(context, scene->getNumVertices() * sizeof(uint64_t) * sortBufferSizeMultiplier,
                                      false, 0, "sortKBufferEven");
    sortKBufferOdd = Buffer::storage(context, scene->getNumVertices() * sizeof(uint64_t) * sortBufferSizeMultiplier,
                                     false, 0, "sortKBufferOdd");
    sortVBufferEven = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t) * sortBufferSizeMultiplier,
                                      false, 0, "sortVBufferEven");
    sortVBufferOdd = Buffer::storage(context, scene->getNumVertices() * sizeof(uint32_t) * sortBufferSizeMultiplier,
                                     false, 0, "sortVBufferOdd");

    uint32_t globalInvocationSize = scene->getNumVertices() * sortBufferSizeMultiplier / numRadixSortBlocksPerWorkgroup;
    uint32_t remainder = scene->getNumVertices() * sortBufferSizeMultiplier % numRadixSortBlocksPerWorkgroup;
    globalInvocationSize += remainder > 0 ? 1 : 0;

    auto numWorkgroups = (globalInvocationSize + 256 - 1) / 256;

    sortHistBuffer = Buffer::storage(context, numWorkgroups * 256 * sizeof(uint32_t), false);

    sortHistPipeline = std::make_shared<ComputePipeline>(
        context, std::make_shared<Shader>(context, "hist", SPV_HIST, SPV_HIST_len));
    sortPipeline = std::make_shared<ComputePipeline>(
        context, std::make_shared<Shader>(context, "sort", SPV_SORT, SPV_SORT_len));

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
    spdlog::debug("Creating preprocess sort pipeline");
    preprocessSortPipeline = std::make_shared<ComputePipeline>(
        context, std::make_shared<Shader>(context, "preprocess_sort", SPV_PREPROCESS_SORT, SPV_PREPROCESS_SORT_len));
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
    spdlog::debug("Creating tile boundary pipeline");
    auto [width, height] = swapchain->swapchainExtent;
    auto tileX = (width + 16 - 1) / 16;
    auto tileY = (height + 16 - 1) / 16;
    tileBoundaryBuffer = Buffer::storage(context, tileX * tileY * sizeof(uint32_t) * 2, false);

    tileBoundaryPipeline = std::make_shared<ComputePipeline>(
        context, std::make_shared<Shader>(context, "tile_boundary", SPV_TILE_BOUNDARY, SPV_TILE_BOUNDARY_len));
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
    spdlog::debug("Creating render pipeline");
    renderPipeline = std::make_shared<ComputePipeline>(
        context, std::make_shared<Shader>(context, "render", SPV_RENDER, SPV_RENDER_len));
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
    for (auto& image: swapchain->swapchainImages) {
        outputSet->bindImageToDescriptorSet(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute,
                                            image);
    }
    outputSet->build();
    renderPipeline->addDescriptorSet(0, inputSet);
    renderPipeline->addDescriptorSet(1, outputSet);
    renderPipeline->addPushConstant(vk::ShaderStageFlagBits::eCompute, 0, sizeof(uint32_t) * 2);
    renderPipeline->build();
}

void Renderer::draw() {
    auto ret = context->device->waitForFences(inflightFences[0].get(), VK_TRUE, UINT64_MAX);
    if (ret != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence");
    }
    context->device->resetFences(inflightFences[0].get());

    auto res = context->device->acquireNextImageKHR(swapchain->swapchain.get(), UINT64_MAX,
                                                    swapchain->imageAvailableSemaphores[0].get(),
                                                    nullptr, &currentImageIndex);
    if (res == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapchain();
        return;
    } else if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

startOfRenderLoop:
    handleInput();

    updateUniforms();

    auto submitInfo = vk::SubmitInfo{}.setCommandBuffers(preprocessCommandBuffer.get());
    context->queues[VulkanContext::Queue::COMPUTE].queue.submit(submitInfo, inflightFences[0].get());

    ret = context->device->waitForFences(inflightFences[0].get(), VK_TRUE, UINT64_MAX);
    if (ret != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence");
    }
    context->device->resetFences(inflightFences[0].get());

    if (!recordRenderCommandBuffer(0)) {
        goto startOfRenderLoop;
    }
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eComputeShader;
    submitInfo = vk::SubmitInfo{}.setWaitSemaphores(swapchain->imageAvailableSemaphores[0].get())
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

    try {
        ret = context->queues[VulkanContext::Queue::PRESENT].queue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError& e) {
        recreateSwapchain();
        return;
    }

    if (ret == vk::Result::eErrorOutOfDateKHR || ret == vk::Result::eSuboptimalKHR) {
        recreateSwapchain();
    } else if (ret != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swapchain image");
    }
}

void Renderer::run() {
    while (running) {
        if (!window->tick()) {
            break;
        }

        draw();

        auto now = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFpsTime).count();
        if (diff > 1000) {
            spdlog::debug("FPS: {}", fpsCounter);
            fpsCounter = 0;
            lastFpsTime = now;
        } else {
            fpsCounter++;
        }

        retrieveTimestamps();
    }

    context->device->waitIdle();
}

void Renderer::stop() {
    // wait till device is idle
    running = false;

    context->device->waitIdle();
}

void Renderer::createCommandPool() {
    spdlog::debug("Creating command pool");
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = context->queues[VulkanContext::Queue::COMPUTE].queueFamily;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    commandPool = context->device->createCommandPoolUnique(poolInfo, nullptr);
}

void Renderer::recordPreprocessCommandBuffer() {
    spdlog::debug("Recording preprocess command buffer");
    if (!preprocessCommandBuffer) {
        vk::CommandBufferAllocateInfo allocateInfo = {commandPool.get(), vk::CommandBufferLevel::ePrimary, 1};
        auto buffers = context->device->allocateCommandBuffersUnique(allocateInfo);
        preprocessCommandBuffer = std::move(buffers[0]);
    }
    preprocessCommandBuffer->reset();

    auto numGroups = (scene->getNumVertices() + 255) / 256;

    preprocessCommandBuffer->begin(vk::CommandBufferBeginInfo{});

    preprocessCommandBuffer->resetQueryPool(context->queryPool.get(), 0, 12);

    preprocessPipeline->bind(preprocessCommandBuffer, 0, 0);
    preprocessCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                            queryManager->registerQuery("preprocess_start"));
    preprocessCommandBuffer->dispatch(numGroups, 1, 1);
    tileOverlapBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());

    vk::BufferCopy copyRegion = {0, 0, tileOverlapBuffer->size};
    preprocessCommandBuffer->copyBuffer(tileOverlapBuffer->buffer, prefixSumPingBuffer->buffer, 1, &copyRegion);

    prefixSumPingBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());

    preprocessCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                            queryManager->registerQuery("preprocess_end"));

    prefixSumPipeline->bind(preprocessCommandBuffer, 0, 0);
    preprocessCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                            queryManager->registerQuery("prefix_sum_start"));
    const auto iters = static_cast<uint32_t>(std::ceil(std::log2(static_cast<float>(scene->getNumVertices()))));
    for (uint32_t timestep = 0; timestep <= iters; timestep++) {
        preprocessCommandBuffer->pushConstants(prefixSumPipeline->pipelineLayout.get(),
                                               vk::ShaderStageFlagBits::eCompute, 0,
                                               sizeof(uint32_t), &timestep);
        preprocessCommandBuffer->dispatch(numGroups, 1, 1);

        if (timestep % 2 == 0) {
            prefixSumPongBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());
            prefixSumPingBuffer->computeReadWriteBarrier(preprocessCommandBuffer.get());
        } else {
            prefixSumPingBuffer->computeWriteReadBarrier(preprocessCommandBuffer.get());
            prefixSumPongBuffer->computeReadWriteBarrier(preprocessCommandBuffer.get());
        }
    }

    auto totalSumRegion = vk::BufferCopy{(scene->getNumVertices() - 1) * sizeof(uint32_t), 0, sizeof(uint32_t)};
    if (iters % 2 == 0) {
        preprocessCommandBuffer->copyBuffer(prefixSumPingBuffer->buffer, totalSumBufferHost->buffer, 1,
                                            &totalSumRegion);
    } else {
        preprocessCommandBuffer->copyBuffer(prefixSumPongBuffer->buffer, totalSumBufferHost->buffer, 1,
                                            &totalSumRegion);
    }

    preprocessCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                        queryManager->registerQuery("prefix_sum_end"));

    preprocessCommandBuffer->end();
}


bool Renderer::recordRenderCommandBuffer(uint32_t currentFrame) {
    if (!renderCommandBuffer) {
        renderCommandBuffer = std::move(context->device->allocateCommandBuffersUnique(
            vk::CommandBufferAllocateInfo(commandPool.get(), vk::CommandBufferLevel::ePrimary, 1))[0]);
    }

    uint32_t numInstances = totalSumBufferHost->readOne<uint32_t>();
    // spdlog::debug("Num instances: {}", numInstances);
    guiManager.pushTextMetric("instances", numInstances);
    if (numInstances > scene->getNumVertices() * sortBufferSizeMultiplier) {
        auto old = sortBufferSizeMultiplier;
        while (numInstances > scene->getNumVertices() * sortBufferSizeMultiplier) {
            sortBufferSizeMultiplier++;
        }
        spdlog::info("Reallocating sort buffers. {} -> {}", old, sortBufferSizeMultiplier);
        sortKBufferEven->realloc(scene->getNumVertices() * sizeof(uint64_t) * sortBufferSizeMultiplier);
        sortKBufferOdd->realloc(scene->getNumVertices() * sizeof(uint64_t) * sortBufferSizeMultiplier);
        sortVBufferEven->realloc(scene->getNumVertices() * sizeof(uint32_t) * sortBufferSizeMultiplier);
        sortVBufferOdd->realloc(scene->getNumVertices() * sizeof(uint32_t) * sortBufferSizeMultiplier);

        uint32_t globalInvocationSize = scene->getNumVertices() * sortBufferSizeMultiplier /
                                        numRadixSortBlocksPerWorkgroup;
        uint32_t remainder = scene->getNumVertices() * sortBufferSizeMultiplier % numRadixSortBlocksPerWorkgroup;
        globalInvocationSize += remainder > 0 ? 1 : 0;

        auto numWorkgroups = (globalInvocationSize + 256 - 1) / 256;

        sortHistBuffer->realloc(numWorkgroups * 256 * sizeof(uint32_t));

        recordPreprocessCommandBuffer();
        return false;
    }

    renderCommandBuffer->reset({});
    renderCommandBuffer->begin(vk::CommandBufferBeginInfo{});

#ifdef VKGS_ENABLE_METAL
    if (numInstances == 0 && __APPLE__) {
        renderCommandBuffer->end();
        return true;
    }
#endif

    vertexAttributeBuffer->computeWriteReadBarrier(renderCommandBuffer.get());

    const auto iters = static_cast<uint32_t>(std::ceil(std::log2(static_cast<float>(scene->getNumVertices()))));
    auto numGroups = (scene->getNumVertices() + 255) / 256;
    preprocessSortPipeline->bind(renderCommandBuffer, 0, iters % 2 == 0 ? 0 : 1);
    renderCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                            queryManager->registerQuery("preprocess_sort_start"));
    uint32_t tileX = (swapchain->swapchainExtent.width + 16 - 1) / 16;
    // assert(tileX == 50);
    renderCommandBuffer->pushConstants(preprocessSortPipeline->pipelineLayout.get(),
                                           vk::ShaderStageFlagBits::eCompute, 0,
                                           sizeof(uint32_t), &tileX);
    renderCommandBuffer->dispatch(numGroups, 1, 1);

    sortKBufferEven->computeWriteReadBarrier(renderCommandBuffer.get());
    renderCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                            queryManager->registerQuery("preprocess_sort_end"));

    // std::cout << "Num instances: " << numInstances << std::endl;

    assert(numInstances <= scene->getNumVertices() * sortBufferSizeMultiplier);
    renderCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                                queryManager->registerQuery("sort_start"));
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
        } else {
            sortKBufferEven->computeWriteReadBarrier(renderCommandBuffer.get());
            sortVBufferEven->computeWriteReadBarrier(renderCommandBuffer.get());
        }
    }
    renderCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                                queryManager->registerQuery("sort_end"));

    renderCommandBuffer->fillBuffer(tileBoundaryBuffer->buffer, 0, VK_WHOLE_SIZE, 0);

    Utils::BarrierBuilder().queueFamilyIndex(context->queues[VulkanContext::Queue::COMPUTE].queueFamily)
            .addBufferBarrier(tileBoundaryBuffer, vk::AccessFlagBits::eTransferWrite,
                              vk::AccessFlagBits::eShaderWrite)
            .build(renderCommandBuffer.get(), vk::PipelineStageFlagBits::eTransfer,
                   vk::PipelineStageFlagBits::eComputeShader);

    // Since we have 64 bit keys, the sort result is always in the even buffer
    tileBoundaryPipeline->bind(renderCommandBuffer, 0, 0);
    renderCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                        queryManager->registerQuery("tile_boundary_start"));
    renderCommandBuffer->pushConstants(tileBoundaryPipeline->pipelineLayout.get(),
                                       vk::ShaderStageFlagBits::eCompute, 0,
                                       sizeof(uint32_t), &numInstances);
    renderCommandBuffer->dispatch((numInstances + 255) / 256, 1, 1);

    tileBoundaryBuffer->computeWriteReadBarrier(renderCommandBuffer.get());
    renderCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                        queryManager->registerQuery("tile_boundary_end"));

    renderPipeline->bind(renderCommandBuffer, 0, std::vector<uint32_t>{0, currentImageIndex});
    renderCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                        queryManager->registerQuery("render_start"));
    auto [width, height] = swapchain->swapchainExtent;
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
    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    if (configuration.enableGui) {
        imageMemoryBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        renderCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                                             vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                             vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
    } else {
        imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
        renderCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                                             vk::PipelineStageFlagBits::eBottomOfPipe,
                                             vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
    }
    renderCommandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, context->queryPool.get(),
                                        queryManager->registerQuery("render_end"));

    if (configuration.enableGui) {
        imguiManager->draw(renderCommandBuffer.get(), currentImageIndex, std::bind(&GUIManager::buildGui, &guiManager));

        imageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;

        renderCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                             vk::PipelineStageFlagBits::eComputeShader,
                                             vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
    }
    renderCommandBuffer->end();

    return true;
}

void Renderer::updateUniforms() {
    UniformBuffer data{};
    auto [width, height] = swapchain->swapchainExtent;
    data.width = width;
    data.height = height;
    data.camera_position = glm::vec4(camera.position, 1.0f);

    auto rotation = glm::mat4_cast(camera.rotation);
    auto translation = glm::translate(glm::mat4(1.0f), camera.position);
    auto view = glm::inverse(translation * rotation);

    float tan_fovx = std::tan(glm::radians(camera.fov) / 2.0);
    float tan_fovy = tan_fovx * static_cast<float>(height) / static_cast<float>(width);
    data.view_mat = view;
    data.proj_mat = glm::perspective(std::atan(tan_fovy) * 2.0f,
                                     static_cast<float>(width) / static_cast<float>(height),
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
    data.tan_fovx = tan_fovx;
    data.tan_fovy = tan_fovy;
    uniformBuffer->upload(&data, sizeof(UniformBuffer), 0);
}

Renderer::~Renderer() {
}
