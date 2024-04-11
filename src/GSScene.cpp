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

#include "third_party/json.hpp"

#include <glm/gtc/quaternion.hpp>

using json = nlohmann::json;

struct VertexStorage {
    glm::vec3 position;
    glm::vec3 normal;
    float shs[48];
    float opacity;
    glm::vec3 scale;
    glm::vec4 rotation;
};

void GSScene::parseCameras(std::filesystem::path::iterator::reference path) {
    std::ifstream camerasFile(path);
    json data = json::parse(camerasFile);

    for (const auto &[key, value]: data.items()) {
        auto position = value["position"];
        auto rotation = value["rotation"];
        auto rotMat = glm::mat3(
            glm::vec3(rotation[0][0], rotation[1][0], rotation[2][0]),
            -glm::vec3(rotation[0][1], rotation[1][1], rotation[2][1]),
            -glm::vec3(rotation[0][2], rotation[1][2], rotation[2][2])
        );
        cameras.emplace_back(glm::vec3(position[0], position[1], position[2]), glm::quat(rotMat));
    }
}

void GSScene::findPlyFile(std::filesystem::path::iterator::reference path) {
    // ply files are under point_cloud/iteration_*/point_cloud.ply
    // find the latest iteration
    std::filesystem::path latestIteration;
    int latestIterationNumber = -1;
    for (const auto &entry: std::filesystem::directory_iterator(path / "point_cloud")) {
        if (entry.is_directory()) {
            auto iteration = entry.path().filename().string();
            try {
                // substr(10) to remove "iteration_" prefix
                int iterationNumber = std::stoi(iteration.substr(10));
                if (iterationNumber > latestIterationNumber) {
                    latestIterationNumber = iterationNumber;
                    latestIteration = entry.path();
                }
            } catch (std::invalid_argument &e) {
                // ignore
            }
        }
    }

    filename = latestIteration / "point_cloud.ply";

    spdlog::info("Found ply file: {}", filename);

    if (!std::filesystem::exists(filename)) {
        spdlog::critical("File does not exist: {}", filename);
        throw std::runtime_error("File does not exist: " + filename);
    }
}

void GSScene::load(const std::shared_ptr<VulkanContext>&context) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // Check if the file is a folder
    auto path = std::filesystem::path(filename);
    if (std::filesystem::is_directory(filename)) {
        // Check if cameras.json exists
        auto camerasPath = path / "cameras.json";
        if (std::filesystem::exists(camerasPath)) {
            spdlog::info("Dataset folder detected, loading cameras.json");

            parseCameras(camerasPath);
        } else {
            spdlog::critical("cameras.json not found in dataset folder: {}", camerasPath.string());
            throw std::runtime_error("cameras.json not found in dataset folder: " + camerasPath.string());
        }
        findPlyFile(path);
    }

    std::ifstream plyFile(filename, std::ios::binary);
    loadPlyHeader(plyFile);

    vertexBuffer = createBuffer(context, header.numVertices * sizeof(Vertex));
    auto vertexStagingBuffer = Buffer::staging(context, header.numVertices * sizeof(Vertex));
    auto* vertexStagingBufferPtr = static_cast<Vertex *>(vertexStagingBuffer->allocation_info.pMappedData);

    std::vector<Vertex> vertices(header.numVertices);

    auto posMin = glm::vec3(std::numeric_limits<float>::max());
    auto posMax = glm::vec3(std::numeric_limits<float>::min());

    for (auto i = 0; i < header.numVertices; i++) {
        static_assert(sizeof(VertexStorage) == 62 * sizeof(float));
        assert(plyFile.is_open());
        assert(!plyFile.eof());
        VertexStorage vertexStorage;
        plyFile.read(reinterpret_cast<char *>(&vertexStorage), sizeof(VertexStorage));
        vertices[i].position = glm::vec4(vertexStorage.position, 1.0f);
        posMin = glm::min(posMin, vertexStorage.position);
        posMax = glm::max(posMax, vertexStorage.position);
        // verteces[i].normal = glm::vec4(vertexStorage.normal, 0.0f);
        vertices[i].scale_opacity = glm::vec4(glm::exp(vertexStorage.scale), 1.0f / (1.0f + std::exp(-vertexStorage.opacity)));
        vertices[i].rotation = normalize(vertexStorage.rotation);
        // memcpy(verteces[i].shs, vertexStorage.shs, 48 * sizeof(float));
        vertices[i].shs[0] = vertexStorage.shs[0];
        vertices[i].shs[1] = vertexStorage.shs[1];
        vertices[i].shs[2] = vertexStorage.shs[2];
        auto SH_N = 16;
        for (auto j = 1; j < SH_N; j++) {
            vertices[i].shs[j * 3 + 0] = vertexStorage.shs[(j - 1) + 3];
            vertices[i].shs[j * 3 + 1] = vertexStorage.shs[(j - 1) + SH_N + 2];
            vertices[i].shs[j * 3 + 2] = vertexStorage.shs[(j - 1) + SH_N * 2 + 1];
        }
        assert(vertexStorage.normal.x == 0.0f);
        assert(vertexStorage.normal.y == 0.0f);
        assert(vertexStorage.normal.z == 0.0f);
    }

    std::sort(vertices.begin(), vertices.end(), [&](const Vertex &a, const Vertex &b) {
        auto relAPos = (glm::vec3(a.position) - posMin) / (posMax - posMin);
        auto relBPos = (glm::vec3(b.position) - posMin) / (posMax - posMin);
        auto scaledRelAPos = static_cast<float>((1 << 21) - 1) * relAPos;
        auto scaledRelBPos = static_cast<float>((1 << 21) - 1) * relBPos;

        uint64_t codeA = 0;
        uint64_t codeB = 0;
        for (auto i = 0; i < 21; i++) {
            codeA |= (static_cast<uint64_t>(scaledRelAPos.x) & (1 << i)) << (i * 2 + 0);
            codeA |= (static_cast<uint64_t>(scaledRelAPos.y) & (1 << i)) << (i * 2 + 1);
            codeA |= (static_cast<uint64_t>(scaledRelAPos.z) & (1 << i)) << (i * 2 + 2);

            codeB |= (static_cast<uint64_t>(scaledRelBPos.x) & (1 << i)) << (i * 2 + 0);
            codeB |= (static_cast<uint64_t>(scaledRelBPos.y) & (1 << i)) << (i * 2 + 1);
            codeB |= (static_cast<uint64_t>(scaledRelBPos.z) & (1 << i)) << (i * 2 + 2);
        }

        return codeA < codeB;
    });

    // copy vertices to staging buffer
    memcpy(vertexStagingBufferPtr, vertices.data(), header.numVertices * sizeof(Vertex));

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
