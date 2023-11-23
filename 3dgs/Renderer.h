#ifndef RENDERER_H
#define RENDERER_H

#define GLM_SWIZZLE

#include <atomic>

#include "../vulkan/Window.h"
#include "GSScene.h"
#include "../vulkan/pipelines/ComputePipeline.h"
#include "../vulkan/Swapchain.h"
#include <glm/gtc/quaternion.hpp>
#include <radix_sort/platforms/vk/radix_sort_vk.h>

struct RendererConfiguration {
    bool enableVulkanValidationLayers = false;
    std::optional<uint8_t> physicalDeviceId = std::nullopt;
    bool immediateSwapchain = false;
    std::string scene;

    float fov = 45.0f;
    float near = 0.2f;
    float far = 1000.0f;
};

class Renderer {
public:
    struct alignas(16) UniformBuffer {
        glm::vec4 camera_position;
        glm::mat4 proj_mat;
        glm::mat4 view_mat;
        uint32_t width;
        uint32_t height;
        float tan_fovx;
        float tan_fovy;
    };

    struct VertexAttributeBuffer {
        glm::vec4 conic_opacity;
        glm::vec4 color_radii;
        glm::uvec4 aabb;
        glm::vec2 uv;
        float depth;
        float __padding[1];
    };

    struct Camera {
        glm::vec3 position;
        glm::quat rotation;
        float fov;
        float nearPlane;
        float farPlane;
    };

    struct RadixSortPushConstants {
        uint32_t g_num_elements; // == NUM_ELEMENTS
        uint32_t g_shift; // (*)
        uint32_t g_num_workgroups; // == NUMBER_OF_WORKGROUPS as defined in the section above
        uint32_t g_num_blocks_per_workgroup; // == NUM_BLOCKS_PER_WORKGROUP
    };

    explicit Renderer(RendererConfiguration configuration);

    void initialize();

    void handleInput();

    void run();

    ~Renderer();
private:
    RendererConfiguration configuration;
    std::shared_ptr<Window> window;
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<GSScene> scene;

    std::shared_ptr<ComputePipeline> preprocessPipeline;
    std::shared_ptr<ComputePipeline> renderPipeline;
    std::shared_ptr<ComputePipeline> prefixSumPipeline;
    std::shared_ptr<ComputePipeline> preprocessSortPipeline;
    std::shared_ptr<ComputePipeline> sortHistPipeline;
    std::shared_ptr<ComputePipeline> sortPipeline;
    std::shared_ptr<ComputePipeline> tileBoundaryPipeline;

    std::shared_ptr<Buffer> uniformBuffer;
    std::shared_ptr<Buffer> vertexAttributeBuffer;
    std::shared_ptr<Buffer> tileOverlapBuffer;
    std::shared_ptr<Buffer> prefixSumPingBuffer;
    std::shared_ptr<Buffer> prefixSumPongBuffer;
    std::shared_ptr<Buffer> sortKBufferEven;
    std::shared_ptr<Buffer> sortKBufferOdd;
    std::shared_ptr<Buffer> sortHistBuffer;
    std::shared_ptr<Buffer> totalSumBufferHost;
    std::shared_ptr<Buffer> tileBoundaryBuffer;
    std::shared_ptr<Buffer> sortVBufferEven;
    std::shared_ptr<Buffer> sortVBufferOdd;

    std::shared_ptr<DescriptorSet> inputSet;

    std::atomic<bool> running = true;

    std::vector<vk::UniqueFence> inflightFences;

    std::shared_ptr<Swapchain> swapchain;


    Camera camera {
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
        .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        .fov = 45.0f,
        .nearPlane = 0.1f,
        .farPlane = 1000.0f
    };

    vk::UniqueCommandPool commandPool;

    vk::UniqueCommandBuffer preprocessCommandBuffer;
    vk::UniqueCommandBuffer renderCommandBuffer;

    const radix_sort_vk_target_t* radixSortTarget;
    radix_sort_vk_t* radixSortPipeline;
    radix_sort_vk_memory_requirements_t radixSortRequirements = {};

    uint32_t currentImageIndex;

    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;

    uint32_t numRadixSortBlocksPerWorkgroup = 32;

    int fpsCounter = 0;
    std::chrono::high_resolution_clock::time_point lastFpsTime = std::chrono::high_resolution_clock::now();

    void initializeVulkan();

    void loadSceneToGPU();

    void createPreprocessPipeline();

    void createPrefixSumPipeline();

    void createRadixSortPipeline();

    void createPreprocessSortPipeline();

    void createTileBoundaryPipeline();

    void createRenderPipeline();

    void recordPreprocessCommandBuffer();

    void recordRenderCommandBuffer(uint32_t currentFrame);

    void createCommandPool();

    void updateUniforms();
};


#endif //RENDERER_H
