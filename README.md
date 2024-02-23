# VulkanSplatting
VulkanSplatting is an (not-yet-highly-) optimized, cross-platform implementation of [Gaussian Splatting](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/) using the [Vulkan API](https://www.khronos.org/vulkan/) and compute pipelines.

[![Windows + Linux](https://github.com/shg8/VulkanSplatting/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=main)](https://github.com/shg8/VulkanSplatting/actions/workflows/cmake-multi-platform.yml)

![VulkanSplatting Demo macOS](https://github.com/shg8/VulkanSplatting/assets/38004233/66542056-ce30-4998-a612-dd4f6792599e)

## TODO
The goal of this project is to provide a go-to implementation for high performance rendering of point-based radiance fields that works on all platforms, but we need your help! Please feel free to open an issue if you have any ideas or are interested in contributing.

- [ ] Better controls and GUI on GLFW
- [ ] Implement SOTA parallel radix sort for sorting Gaussian instances
- [ ] Use Vulkan subgroups to batch Gaussian retrievals at the warp level
- [ ] OpenXR support
- [ ] Apps for iOS and visionOS
- [ ] Training

## Command Line Usage
```
./vulkan_splatting [options] <input_file.ply>
```

### Options

- `--validation`: Enable Vulkan validation layers. This is useful for debugging and development purposes.

- `-v`, `--verbose`: Enable verbose logging. This will output more detailed logs, which can be helpful for troubleshooting issues.

- `-d`, `--device <index>`: Select physical device by index. This allows you to specify which physical device (e.g., GPU) to use for rendering.

- `-i`, `--immediate-swapchain`: Set swapchain mode to immediate (VK_PRESENT_MODE_IMMEDIATE_KHR). This can affect the presentation mode of the swapchain.

- `--no-gui`: Disable GUI. This runs the application in a headless mode, without a graphical user interface.

- `scene <path>`: Path to the scene file. This is a positional argument that specifies the path to the .ply file that contains the 3D scene to be rendered.

## Building
### Linux
VulkanSplatting requires the following dependencies:

`Vulkan headers, Vulkan validation layers, glslangValidator, glfw, glm`

The easiest way to install the first three is through the [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/). Alternatively, you can install the corresponding packages from your distro. For Ubuntu, the packages to install are `vulkan-headers, vulkan-validationlayers, glslang-dev, libglfw3-dev, libglm-dev`.

### Windows
After installing Vulkan SDK, set the `VULKAN_SDK` environmental variable to the install path. Alternatively, pass `-DVULKAN_SDK=\INSTALL\LOCATION\OF\YOUR\SDK` to CMake when configuring.

A full CMake configure command is as follows:
```
mkdir .\VulkanSplatting\build
cmake -DCMAKE_BUILD_TYPE=Release -DVULKAN_SDK=\INSTALL\LOCATION\OF\YOUR\SDK -S .\VulkanSplatting -B .\VulkanSplatting\build
```

### macOS
After installing the Vulkan SDK, please proceed with CMake configuration and build steps as usual.
