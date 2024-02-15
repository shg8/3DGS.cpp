# VulkanSplatting
VulkanSplatting is an (not-yet-highly-) optimized, cross-platform implementation of [Gaussian Splatting](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/) using the [Vulkan API](https://www.khronos.org/vulkan/) and compute pipelines.

## Building
### Linux
VulkanSplatting requires the following dependencies:
`Vulkan headers, Vulkan validation layers, glslangValidator, glfw, glm`

The easiest way to install the first three is through the [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/). Alternatively, you can install the corresponding packages from your distro. For Ubuntu, the packages to install are `vulkan-headers, vulkan-validationlayers, glslang-dev, libglfw3-dev, libglm-dev`.

### Windows
After installing [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) with all the required components (see the Linux section), set the `VULKAN_SDK` environmental variable to the install path. Alternatively, pass "-DVULKAN_SDK=\INSTALL\LOCATION\OF\YOUR\SDK" to CMake.

A full CMake configure command is as follows:
```
mkdir .\VulkanSplatting\build
cmake -DCMAKE_BUILD_TYPE=Release -DVULKAN_SDK=\INSTALL\LOCATION\OF\YOUR\SDK -S .\VulkanSplatting -B .\VulkanSplatting\build
```

Only Ninja has been tested as a generator on Windows, but other generators should work as well. To use ninja, add `-G Ninja` to the configure command and make sure that the binary is available in your PATH.


## Command Line Usage
```
./vulkan_splatting [options] <input_file.ply>
```

### Options
- `-h`, `--help`: Show help message and exit
- `-v`, `--validation-layers`: Enable Vulkan validation layers
- `-d`, `--device`: Select physical device by index
- `-i`, `--immediate-swapchain`: Set swapchain mode to immediate (VK_PRESENT_MODE_IMMEDIATE_KHR)
