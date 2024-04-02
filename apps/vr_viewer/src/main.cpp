#include "VRViewer.h"
#include "args.hxx"
#include "spdlog/spdlog.h"
#include <filesystem>

int main(int argc, char** argv) {
    spdlog::set_pattern("[%H:%M:%S] [%^%L%$] %v");

    args::ArgumentParser parser("3DGS VR Viewer");
    args::HelpFlag helpFlag{parser, "help", "Display this help menu", {'h', "help"}};
    args::Flag verboseFlag{parser, "verbose", "Enable verbose logging", {'v', "verbose"}};
    args::ValueFlag<uint8_t> physicalDeviceIdFlag{
        parser, "physical-device", "Select physical device by index", {'d', "device"}
    };
    args::Flag immediateSwapchainFlag{
        parser, "immediate-swapchain", "Set swapchain mode to immediate (VK_PRESENT_MODE_IMMEDIATE_KHR)",
        {'i', "immediate-swapchain"}
    };
    args::Positional<std::string> scenePath{parser, "scene", "Path to scene file", "scene.ply"};

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Completion& e)
    {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        std::cout << e.what() << std::endl;
        std::cout << parser;
        return 1;
    }

    if (args::get(verboseFlag))
    {
        spdlog::set_level(spdlog::level::debug);
    }

    VRViewer::Configuration config {args::get(scenePath)};

    // check that the scene file exists
    if (!std::filesystem::exists(config.scenePath))
    {
        spdlog::critical("File does not exist: {}", config.scenePath);
        return 0;
    }

    VRViewer viewer {config};
    viewer.run();
    return 0;
}
