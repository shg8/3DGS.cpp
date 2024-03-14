//
// Created by steven on 11/30/23.
//

#include <fstream>
#include "GSScene.h"

#include <random>
#include "shaders.h"

#include "vulkan/Utils.h"
#include "vulkan/DescriptorSet.h"
#include "vulkan/pipelines/ComputePipeline.h"
#include "spdlog/spdlog.h"
#include "vulkan/Shader.h"

struct VertexStorage {
    glm::vec3 position;
    glm::vec3 normal;
    float shs[48];
    float opacity;
    glm::vec3 scale;
    glm::vec4 rotation;
};

void GSScene::load(const std::shared_ptr<VulkanContext>&context) {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::ifstream plyFile(filename, std::ios::binary);
    loadPlyHeader(plyFile);

    vertexBuffer = createBuffer(context, header.numVertices * sizeof(Vertex));
    auto vertexStagingBuffer = Buffer::staging(context, header.numVertices * sizeof(Vertex));
    auto* verteces = static_cast<Vertex *>(vertexStagingBuffer->allocation_info.pMappedData);

    for (auto i = 0; i < header.numVertices; i++) {
        static_assert(sizeof(VertexStorage) == 62 * sizeof(float));
        assert(plyFile.is_open());
        assert(!plyFile.eof());
        VertexStorage vertexStorage;
        plyFile.read(reinterpret_cast<char *>(&vertexStorage), sizeof(VertexStorage));
        verteces[i].position = glm::vec4(vertexStorage.position, 1.0f);
        // verteces[i].normal = glm::vec4(vertexStorage.normal, 0.0f);
        verteces[i].scale_opacity = glm::vec4(glm::exp(vertexStorage.scale), 1.0f / (1.0f + std::exp(-vertexStorage.opacity)));
        verteces[i].rotation = normalize(vertexStorage.rotation);
        // memcpy(verteces[i].shs, vertexStorage.shs, 48 * sizeof(float));
        verteces[i].shs[0] = vertexStorage.shs[0];
        verteces[i].shs[1] = vertexStorage.shs[1];
        verteces[i].shs[2] = vertexStorage.shs[2];
        auto SH_N = 16;
        for (auto j = 1; j < SH_N; j++) {
            verteces[i].shs[j * 3 + 0] = vertexStorage.shs[(j - 1) + 3];
            verteces[i].shs[j * 3 + 1] = vertexStorage.shs[(j - 1) + SH_N + 2];
            verteces[i].shs[j * 3 + 2] = vertexStorage.shs[(j - 1) + SH_N * 2 + 1];
        }
        assert(vertexStorage.normal.x == 0.0f);
        assert(vertexStorage.normal.y == 0.0f);
        assert(vertexStorage.normal.z == 0.0f);
    }

    vertexBuffer->uploadFrom(vertexStagingBuffer);

    auto endTime = std::chrono::high_resolution_clock::now();
    spdlog::info("Loaded {} in {}ms", filename,
                 std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());

    precomputeCov3D(context);
}

void GSScene::loadTestScene(const std::shared_ptr<VulkanContext>&context) {
    int testObects = 1;
    header.numVertices = testObects;
    vertexBuffer = createBuffer(context, testObects * sizeof(Vertex));
    auto vertexStagingBuffer = Buffer::staging(context, testObects * sizeof(Vertex));
    auto* verteces = static_cast<Vertex *>(vertexStagingBuffer->allocation_info.pMappedData);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posgen(-3.0, 3.0);
    std::uniform_real_distribution<> scalegen(100.0, 5000.0);
    std::uniform_real_distribution<> shsgen(-1.0, 1.0);


    for (auto i = 0; i < testObects; i++) {
        verteces[i].position = glm::vec4(posgen(gen), posgen(gen), posgen(gen), 1.0f);
        // verteces[i].normal = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        verteces[i].scale_opacity = glm::vec4(scalegen(gen), scalegen(gen), scalegen(gen), 0.5f);
        verteces[i].rotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        for (auto j = 0; j < 48; j++) {
            verteces[i].shs[j] = shsgen(gen);
        }
    }

    vertexBuffer->uploadFrom(vertexStagingBuffer);

    precomputeCov3D(context);
}

void GSScene::loadPlyHeader(std::ifstream&plyFile) {
    if (!plyFile.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    bool headerEnd = false;

    while (std::getline(plyFile, line)) {
        std::istringstream iss(line);
        std::string token;

        iss >> token;

        if (token == "ply") {
            // PLY format indicator
        }
        else if (token == "format") {
            iss >> header.format;
        }
        else if (token == "element") {
            iss >> token;

            if (token == "vertex") {
                iss >> header.numVertices;
            }
            else if (token == "face") {
                iss >> header.numFaces;
            }
        }
        else if (token == "property") {
            PlyProperty property;
            iss >> property.type >> property.name;

            if (header.vertexProperties.size() < static_cast<size_t>(header.numVertices)) {
                header.vertexProperties.push_back(property);
            }
            else {
                header.faceProperties.push_back(property);
            }
        }
        else if (token == "end_header") {
            headerEnd = true;
            break;
        }
    }

    if (!headerEnd) {
        throw std::runtime_error("Could not find end of header");
    }
}

std::shared_ptr<Buffer> GSScene::createBuffer(const std::shared_ptr<VulkanContext>&context, size_t i) {
    return std::make_shared<Buffer>(
        context, i, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, false);
}

void GSScene::precomputeCov3D(const std::shared_ptr<VulkanContext>&context) {
    cov3DBuffer = createBuffer(context, header.numVertices * sizeof(float) * 6);

    auto pipeline = std::make_shared<ComputePipeline>(
        context, std::make_shared<Shader>(context, "precomp_cov3d", SPV_PRECOMP_COV3D, SPV_PRECOMP_COV3D_len));

    auto descriptorSet = std::make_shared<DescriptorSet>(context, FRAMES_IN_FLIGHT);
    descriptorSet->bindBufferToDescriptorSet(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             vertexBuffer);
    descriptorSet->bindBufferToDescriptorSet(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute,
                                             cov3DBuffer);
    descriptorSet->build();

    pipeline->addDescriptorSet(0, descriptorSet);
    pipeline->addPushConstant(vk::ShaderStageFlagBits::eCompute, 0, sizeof(float));
    pipeline->build();

    auto commandBuffer = context->beginOneTimeCommandBuffer();
    pipeline->bind(commandBuffer, 0, 0);
    float scaleFactor = 1.0f;
    commandBuffer->pushConstants(pipeline->pipelineLayout.get(), vk::ShaderStageFlagBits::eCompute, 0,
                                 sizeof(float), &scaleFactor);
    int numGroups = (header.numVertices + 255) / 256;
    commandBuffer->dispatch(numGroups, 1, 1);
    context->endOneTimeCommandBuffer(std::move(commandBuffer), VulkanContext::Queue::COMPUTE);

    spdlog::info("Precomputed Cov3D");
}
