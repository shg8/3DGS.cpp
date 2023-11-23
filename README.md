# VulkanSplatting
VulkanSplatting is an (not-yet-highly-) optimized, cross-platform implementation of [Gaussian Splatting](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/) using the [Vulkan API](https://www.khronos.org/vulkan/) using compute pipelines.

## Command Line Usage
```
./vulkan-splatting [options] <input_file.ply>
```
### Options
- `-h`, `--help`: Show help message and exit
- `-v`, `--validation-layers`: Enable Vulkan validation layers
- `-d`, `--device`: Select physical device by index
- `-i`, `--immediate-swapchain`: Set swapchain mode to immediate (VK_PRESENT_MODE_IMMEDIATE_KHR)