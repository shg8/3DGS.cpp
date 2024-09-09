#include "ImguiManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "windowing/GLFWWindow.h"

ImguiManager::ImguiManager(std::shared_ptr<VulkanContext> context, std::shared_ptr<Swapchain> swapchain,
                           std::shared_ptr<Window> window) : context(context), swapchain(swapchain), window(window) {
}

void ImguiManager::createCommandPool() {
}

void ImguiManager::setStyle() {
    auto& colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
    colors[ImGuiCol_MenuBarBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Border
    colors[ImGuiCol_Border] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
    colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.24f};

    // Text
    colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
    colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

    // Headers
    colors[ImGuiCol_Header] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_HeaderActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_CheckMark] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};

    // Popups
    colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.13f, 0.92f};

    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4{0.44f, 0.37f, 0.61f, 0.54f};
    colors[ImGuiCol_SliderGrabActive] = ImVec4{0.74f, 0.58f, 0.98f, 0.54f};

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4{0.13f, 0.13, 0.17, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_FrameBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TabHovered] = ImVec4{0.24, 0.24f, 0.32f, 1.0f};
    colors[ImGuiCol_TabActive] = ImVec4{0.2f, 0.22f, 0.27f, 1.0f};
    colors[ImGuiCol_TabUnfocused] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
    colors[ImGuiCol_ScrollbarGrab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};

    // Seperator
    colors[ImGuiCol_Separator] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
    colors[ImGuiCol_SeparatorHovered] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
    colors[ImGuiCol_SeparatorActive] = ImVec4{0.84f, 0.58f, 1.0f, 1.0f};

    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
    colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.74f, 0.58f, 0.98f, 0.29f};
    colors[ImGuiCol_ResizeGripActive] = ImVec4{0.84f, 0.58f, 1.0f, 0.29f};

    auto& style = ImGui::GetStyle();
    style.TabRounding = 4;
    style.ScrollbarRounding = 9;
    style.WindowRounding = 7;
    style.GrabRounding = 3;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ChildRounding = 4;
}

void ImguiManager::init() {
    commandPool = context->device->createCommandPoolUnique(vk::CommandPoolCreateInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        context->queues[VulkanContext::Queue::GRAPHICS].queueFamily));
    commandBuffer = std::move(
        context->device->allocateCommandBuffersUnique(
            vk::CommandBufferAllocateInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 1))[0]);
    fence = context->device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));

    std::vector<vk::DescriptorPoolSize> poolSizes = {
        {vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},
        {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000},
        {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eStorageBufferDynamic, 1000},
        {vk::DescriptorType::eInputAttachment, 1000}
    };

    vk::DescriptorPoolCreateInfo poolInfo{
        vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        100, static_cast<uint32_t>(poolSizes.size()),
        poolSizes.data()
    };

    descriptorPool = context->device->createDescriptorPoolUnique(poolInfo);

    ImGui::CreateContext();
    auto glfwWindow = std::reinterpret_pointer_cast<GLFWWindow>(window);
    ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow *>(glfwWindow->window), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = context->instance.get();
    init_info.PhysicalDevice = context->physicalDevice;
    init_info.Device = context->device.get();
    init_info.QueueFamily = context->queues[VulkanContext::Queue::GRAPHICS].queueFamily;
    init_info.Queue = context->queues[VulkanContext::Queue::GRAPHICS].queue;
    init_info.DescriptorPool = static_cast<VkDescriptorPool>(descriptorPool.get());
    init_info.MinImageCount = 2;
    init_info.ImageCount = swapchain->imageCount + 1;
    init_info.UseDynamicRendering = true;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{
        {}, 1, &swapchain->swapchainFormat
    };

    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();

    setStyle();
}

void ImguiManager::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
    context->device->resetFences(*fence);
    commandBuffer->reset();
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer->begin(beginInfo);
    function(*commandBuffer);
    commandBuffer->end();
    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffer;
    context->queues[VulkanContext::Queue::GRAPHICS].queue.submit(submitInfo, *fence);
    auto ret = context->device->waitForFences(*fence, VK_TRUE, UINT64_MAX);
    if (ret != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence");
    }
}

void ImguiManager::draw(vk::CommandBuffer commandBuffer, uint32_t currentImageIndex, std::function<void(void)> imguiFunction) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    imguiFunction();
    ImGui::Render();

    vk::RenderingAttachmentInfoKHR attachment_info{
        swapchain->swapchainImages[currentImageIndex]->imageView.get(), vk::ImageLayout::eColorAttachmentOptimal
    };
    vk::RenderingInfoKHR rendering_info{{}, vk::Rect2D{{0, 0}, swapchain->swapchainExtent}, 1, {}, 1, &attachment_info};
    commandBuffer.beginRenderingKHR(rendering_info);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    commandBuffer.endRenderingKHR();
}

ImguiManager::~ImguiManager() {
    // Cleanup
    context->device->waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
