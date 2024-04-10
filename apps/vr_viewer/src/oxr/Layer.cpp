#include "Layer.h"

#include "Utils.h"
#include "spdlog/spdlog.h"
#include "vk_enum_string_helper.h"

namespace OXR {
    Layer::Layer(std::shared_ptr<OXRContext> context, XrCompositionLayerFlags flags, bool depth_enabled) : depthEnabled(
        depth_enabled) {
        createSwapchains(context);
        createProjectionViews(context, flags);
        spdlog::info("Layer created");
    }

    std::pair<std::optional<uint32_t>, bool> Layer::acquireNextImage(int8_t swapchain) {
        XrSwapchainImageAcquireInfo acquireInfo = {XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        auto ret = xrAcquireSwapchainImage(swapchains[swapchain], &acquireInfo, &lastImageAcquired[swapchain]);
        XR_CHECK(ret, "Failed to acquire swapchain image");

        XrSwapchainImageWaitInfo waitInfo = {XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        waitInfo.timeout = INT64_MAX;
        ret = xrWaitSwapchainImage(swapchains[swapchain], &waitInfo);
        XR_CHECK(ret, "Failed to wait for swapchain image");

        return {{lastImageAcquired[swapchain]}, false};
    }

    bool Layer::present(int8_t swapchain, uint32_t imageIndex) {
        assert(imageIndex == lastImageAcquired[swapchain]); // TODO: Remove this check

        XrSwapchainImageReleaseInfo releaseInfo = {XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        XR_CHECK(xrReleaseSwapchainImage(swapchains[swapchain], &releaseInfo), "Failed to release swapchain image");
    }

    void Layer::createSwapchains(std::shared_ptr<OXRContext> context) {
        // Print the swapchain formats
        spdlog::info("Swapchain formats:");
        auto swapchainFormats = getVulkanFormatsForSwapchain(context);
        vk::Format selectedFormat = swapchainFormats[0];
        for (vk::Format format: swapchainFormats) {
            spdlog::info("\t{}", string_VkFormat(static_cast<VkFormat>(format)));
            if (format == vk::Format::eB8G8R8A8Unorm) {
                selectedFormat = format;
            }
        }

        std::vector<XrSwapchainImageVulkanKHR> swapchainImageVectors[2];
        for (int i = 0; i < 2; i++) {
            XrSwapchainCreateInfo swapchainCreateInfo = {XR_TYPE_SWAPCHAIN_CREATE_INFO};
            swapchainCreateInfo.arraySize = 1;
            swapchainCreateInfo.format = static_cast<int64_t>(selectedFormat);
            swapchainCreateInfo.width = context->views[i].recommendedImageRectWidth;
            swapchainCreateInfo.height = context->views[i].recommendedImageRectHeight;
            swapchainCreateInfo.mipCount = 1;
            swapchainCreateInfo.faceCount = 1;
            swapchainCreateInfo.sampleCount = 1;
            swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT |
                                             XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT |
                                             XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
            swapchainCreateInfo.createFlags = 0;

            spdlog::debug("Creating swapchain with format and size: {} {}x{}", string_VkFormat(
                              static_cast<VkFormat>(swapchainCreateInfo.format)), swapchainCreateInfo.width,
                          swapchainCreateInfo.height);

            XR_CHECK(xrCreateSwapchain(context->oxrSession, &swapchainCreateInfo, &swapchains[i]),
                     "Failed to create swapchain");

            uint32_t swapchainLength;
            XR_CHECK(xrEnumerateSwapchainImages(swapchains[i], 0, &swapchainLength, nullptr),
                     "Failed to enumerate swapchain images");

            std::vector<XrSwapchainImageVulkanKHR> swapchainImages(swapchainLength);
            for (uint32_t j = 0; j < swapchainLength; j++) {
                swapchainImages[j] = {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR};
            }

            XR_CHECK(xrEnumerateSwapchainImages(swapchains[i], swapchainLength, &swapchainLength,
                         reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImages.data())),
                     "Failed to enumerate swapchain images");

            swapchainImageVectors[i] = swapchainImages;
            swapchainSize[i] = swapchainLength;
        }

        assert(swapchainSize[0] == swapchainSize[1]);

        images.resize(swapchainSize[0]);
        for (uint32_t i = 0; i < swapchainSize[0]; i++) {
            images[i][0] = swapchainImageVectors[0][i];
            images[i][1] = swapchainImageVectors[1][i];
        }

        if (!depthEnabled) {
            return;
        }

        vk::Format selectedDepthFormat = vk::Format::eUndefined;
        for (vk::Format format: swapchainFormats) {
            if (format == vk::Format::eD32Sfloat) {
                selectedDepthFormat = format;
                break;
            } else if (format == vk::Format::eD16Unorm) {
                selectedDepthFormat = format;
            }
        }

        if (selectedDepthFormat == vk::Format::eUndefined) {
            spdlog::error("No depth format found");
            return;
        }

        spdlog::info("Depth format: {}", string_VkFormat(static_cast<VkFormat>(selectedDepthFormat)));

        for (int i = 0; i < 2; i++) {
            XrSwapchainCreateInfo swapchainCreateInfo = {XR_TYPE_SWAPCHAIN_CREATE_INFO};
            swapchainCreateInfo.arraySize = 1;
            swapchainCreateInfo.format = static_cast<int64_t>(selectedDepthFormat);
            swapchainCreateInfo.width = context->views[i].recommendedImageRectWidth;
            swapchainCreateInfo.height = context->views[i].recommendedImageRectHeight;
            swapchainCreateInfo.mipCount = 1;
            swapchainCreateInfo.faceCount = 1;
            swapchainCreateInfo.sampleCount = 1;
            swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT |
                                             XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            swapchainCreateInfo.createFlags = 0;

            spdlog::debug("Creating depth swapchain with format and size: {} {}x{}", string_VkFormat(
                              static_cast<VkFormat>(swapchainCreateInfo.format)), swapchainCreateInfo.width,
                          swapchainCreateInfo.height);

            XR_CHECK(xrCreateSwapchain(context->oxrSession, &swapchainCreateInfo, &depthSwapchains[i]),
                     "Failed to create depth swapchain");

            uint32_t swapchainLength;
            XR_CHECK(xrEnumerateSwapchainImages(depthSwapchains[i], 0, &swapchainLength, nullptr),
                     "Failed to enumerate depth swapchain images");

            std::vector<XrSwapchainImageVulkanKHR> swapchainImages(swapchainLength);
            for (uint32_t j = 0; j < swapchainLength; j++) {
                swapchainImages[j] = {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR};
            }

            XR_CHECK(xrEnumerateSwapchainImages(depthSwapchains[i], swapchainLength, &swapchainLength,
                         reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImages.data())),
                     "Failed to enumerate depth swapchain images");

            depthImages.resize(swapchainLength);
            for (uint32_t j = 0; j < swapchainLength; j++) {
                depthImages[j][0] = swapchainImages[j];
                depthImages[j][1] = swapchainImages[j];
            }

            depthSwapchainSize[i] = swapchainLength;
        }
    }

    void Layer::createProjectionViews(std::shared_ptr<OXRContext> context, uint64_t flags) {
        for (int i = 0; i < 2; i++) {
            views[i].pose = {.orientation = {0.0f, 0.0f, 0.0f, 1.0f}, .position = {0.0f, 0.0f, 0.0f}};
            views[i].subImage.swapchain = swapchains[i];
            views[i].subImage.imageRect.offset = {0, 0};
            views[i].subImage.imageRect.extent = {
                static_cast<int32_t>(context->views[i].recommendedImageRectWidth),
                static_cast<int32_t>(context->views[i].recommendedImageRectHeight)
            };
            if (depthEnabled) {
                views[i].next = &depthLayer;
                depthLayer.subImage.swapchain = depthSwapchains[i];
                depthLayer.subImage.imageRect.offset = {0, 0};
                depthLayer.subImage.imageRect.extent = {
                    static_cast<int32_t>(context->views[i].recommendedImageRectWidth),
                    static_cast<int32_t>(context->views[i].recommendedImageRectHeight)
                };
            }
        }

        layer.layerFlags = flags;
        layer.space = context->localSpace;
        layer.viewCount = 2;
        layer.views = views;
    }
} // OXR
