#include <filesystem>
#include <iostream>
#include <libenvpp/env.hpp>

#include "3dgs.h"
#include "args.hxx"
#include "spdlog/spdlog.h"

int main(int argc, char** argv) {
    spdlog::set_pattern("[%H:%M:%S] [%^%L%$] %v");

    args::ArgumentParser parser("Vulkan Splatting");
    args::HelpFlag helpFlag{parser, "help", "Display this help menu", {'h', "help"}};
    args::Flag validationLayersFlag{
        parser, "validation-layers", "Enable Vulkan validation layers", {"validation"}
    };
    args::Flag verboseFlag{parser, "verbose", "Enable verbose logging", {'v', "verbose"}};
    args::ValueFlag<uint32_t> physicalDeviceIdFlag{
        parser, "physical-device", "Select physical device by index", {'d', "device"}
    };
    args::Flag immediateSwapchainFlag{
        parser, "immediate-swapchain", "Set swapchain mode to immediate (VK_PRESENT_MODE_IMMEDIATE_KHR)",
        {'i', "immediate-swapchain"}
    };
    args::ValueFlag<uint32_t> widthFlag{parser, "width", "Set window width", {'w', "width"}};
    args::ValueFlag<uint32_t> heightFlag{parser, "height", "Set window height", {'h', "height"}};
    args::Flag noGuiFlag{parser, "no-gui", "Disable GUI", { "no-gui"}};
    args::ValueFlag<std::string> outputFlag{parser, "output", "Output image path for headless rendering", {'o', "output"}};
    args::ValueFlagList<std::string> cameraFlag{parser, "camera", "Camera configuration file path(s)", {'c', "camera"}};
    args::Positional<std::string> scenePath{parser, "scene", "Path to scene file", "scene.ply"};

    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Completion& e) {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help&) {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e) {
        std::cout << e.what() << std::endl;
        std::cout << parser;
        return 1;
    }

    auto pre = env::prefix("VKGS");
    auto validationLayers = pre.register_variable<bool>("VALIDATION_LAYERS");
    auto physicalDeviceId = pre.register_variable<uint8_t>("PHYSICAL_DEVICE");
    auto immediateSwapchain = pre.register_variable<bool>("IMMEDIATE_SWAPCHAIN");
    auto envVars = pre.parse_and_validate();

    if (args::get(verboseFlag)) {
        spdlog::set_level(spdlog::level::debug);
    }

    VulkanSplatting::RendererConfiguration config{
        envVars.get_or(validationLayers, false),
        envVars.get(physicalDeviceId).has_value()
            ? std::make_optional(envVars.get(physicalDeviceId).value())
            : std::nullopt,
        envVars.get_or(immediateSwapchain, false),
        args::get(scenePath)
    };

    // check that the scene file exists
    if (!std::filesystem::exists(config.scene)) {
        spdlog::critical("File does not exist: {}", config.scene);
        return 1;
    }

    if (validationLayersFlag) {
        config.enableVulkanValidationLayers = args::get(validationLayersFlag);
    }

    if (physicalDeviceIdFlag) {
        config.physicalDeviceId = std::make_optional<uint8_t>(static_cast<uint8_t>(args::get(physicalDeviceIdFlag)));
    }

    if (immediateSwapchainFlag) {
        config.immediateSwapchain = args::get(immediateSwapchainFlag);
    }

    if (noGuiFlag) {
        config.enableGui = false;
    } else {
        config.enableGui = true;
    }

    if (outputFlag) {
        config.outputPath = args::get(outputFlag);
        config.enableGui = false;
    }

    // Handle multiple camera configurations
    std::vector<std::string> cameraPaths;
    if (cameraFlag) {
        cameraPaths = args::get(cameraFlag);
        
        // Multiple cameras only make sense with output path
        if (cameraPaths.size() > 1 && !outputFlag) {
            spdlog::critical("Multiple camera configurations can only be used with output path (-o)");
            return 1;
        }
        
        // For single camera, use the existing logic
        if (cameraPaths.size() == 1) {
            config.cameraPath = cameraPaths[0];
        }
    }

    auto width = widthFlag ? args::get(widthFlag) : 1280;
    auto height = heightFlag ? args::get(heightFlag) : 720;

    if (outputFlag) {
        // For headless mode, we still need a window but make it invisible
        config.window = VulkanSplatting::createGlfwWindow("Vulkan Splatting (Headless)", width, height);
    } else {
        config.window = VulkanSplatting::createGlfwWindow("Vulkan Splatting", width, height);
    }

#ifndef DEBUG
    try {
#endif
    // Handle multiple camera configurations
    if (cameraPaths.size() > 1) {
        spdlog::info("Rendering with {} camera configurations", cameraPaths.size());
        
        for (size_t i = 0; i < cameraPaths.size(); ++i) {
            spdlog::info("Rendering with camera configuration {}: {}", i + 1, cameraPaths[i]);
            
            // Create a new config for this camera
            auto cameraConfig = config;
            cameraConfig.cameraPath = cameraPaths[i];
            
            auto renderer = VulkanSplatting(cameraConfig);
            renderer.start();
        }
    } else {
        // Single camera or no camera - use existing logic
        auto renderer = VulkanSplatting(config);
        renderer.start();
    }
#ifndef DEBUG
    } catch (const std::exception& e) {
        spdlog::critical(e.what());
        std::cout << e.what() << std::endl;
        return 1;
    }
#endif
    return 0;
}
