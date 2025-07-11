#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_TYPESAFE_CONVERSION 1

#define FRAMES_IN_FLIGHT 1

#include <optional>
#include <set>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

struct Image {
    vk::Image image;
    vk::UniqueImageView imageView;
    vk::Format format;
    vk::Extent2D extent;
    std::optional<vk::UniqueFramebuffer> framebuffer;

    Image(const vk::Image &image, vk::UniqueImageView &&image_view, vk::Format format,
          const vk::Extent2D &extent, std::optional<vk::UniqueFramebuffer> &&framebuffer = std::nullopt)
            : image(image),
              imageView(std::move(image_view)),
              format(format),
              extent(extent),
              framebuffer(std::move(framebuffer)) {
    }
};

class VulkanContext {
private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
        }
    };

public:
    struct Queue {
        enum Type {
            GRAPHICS,
            COMPUTE,
            PRESENT
        };

        std::set<Type> types;
        uint32_t queueFamily;
        uint32_t queueIndex;
        vk::Queue queue;
    };

    VulkanContext(const std::vector<std::string> &instance_extensions,
                  const std::vector<std::string> &device_extensions, bool validation_layers_enabled);

    VulkanContext(const VulkanContext &) = delete;

    VulkanContext(VulkanContext &&) = delete;

    VulkanContext &operator=(const VulkanContext &) = delete;

    VulkanContext &operator=(VulkanContext &&) = delete;

    void createInstance();

    bool isDeviceSuitable(vk::PhysicalDevice device, std::optional<vk::SurfaceKHR> surface = std::nullopt);

    void selectPhysicalDevice(std::optional<uint8_t> id = std::nullopt,
                              std::optional<vk::SurfaceKHR> surface = std::nullopt);

    VulkanContext::QueueFamilyIndices findQueueFamilies();

    void createQueryPool();

    void createLogicalDevice(vk::PhysicalDeviceFeatures deviceFeatures, vk::PhysicalDeviceVulkan11Features deviceFeatures11, vk::PhysicalDeviceVulkan12Features deviceFeatures12);

    void createDescriptorPool(uint8_t framesInFlight);

    vk::UniqueCommandBuffer beginOneTimeCommandBuffer();

    void endOneTimeCommandBuffer(vk::UniqueCommandBuffer &&commandBuffer, Queue::Type queue);

    virtual ~VulkanContext();

    vk::UniqueInstance instance;
    vk::PhysicalDevice physicalDevice;
    std::optional<vk::UniqueSurfaceKHR> surface;
    vk::UniqueDevice device;
    std::unordered_map<Queue::Type, Queue> queues;
    VmaAllocator allocator;

    vk::UniqueDescriptorPool descriptorPool;
    vk::UniqueQueryPool queryPool;

    bool validationLayersEnabled;
private:
    std::vector<std::string> instanceExtensions;
    std::vector<std::string> deviceExtensions;

    vk::UniqueCommandPool commandPool;

    void setupVma();

    void createCommandPool();
};


#endif //VULKANCONTEXT_H
