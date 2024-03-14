# VulkanSplatting

VulkanSplatting is an (not-yet-) highly optimized, cross-platform implementation
of [Gaussian Splatting](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/) using
the [Vulkan API](https://www.khronos.org/vulkan/) and compute pipelines.

Why Vulkan? We want to democratize the access to high-performance point-based radiance fields.
Existing implementations of Gaussian Splatting are often limited to CUDA, which only runs on NVIDIA GPUs, 
and OpenGL is deprecated on Apple platforms. Additionally, Vulkan's compute capabilities are the closest to CUDA's 
with support for warp-level primitives (subgroups).

[![Windows + Linux](https://github.com/shg8/VulkanSplatting/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=main)](https://github.com/shg8/VulkanSplatting/actions/workflows/cmake-multi-platform.yml)

![VulkanSplatting Demo macOS](https://github.com/shg8/VulkanSplatting/assets/38004233/66542056-ce30-4998-a612-dd4f6792599e)

## Command Line Usage

```
./vulkan_splatting [options] <input_file.ply>
```

### Options

- `--validation`: Enable Vulkan validation layers

- `-v`, `--verbose`: Enable verbose logging

- `-d`, `--device <index>`: Select GPU by index

- `-i`, `--immediate-swapchain`: Set swapchain mode to immediate (VK_PRESENT_MODE_IMMEDIATE_KHR)

- `-w`, `--width <width>`: Set window width

- `-h`, `--height <height>`: Set window height

- `--no-gui`: Disable GUI

## Building

### Linux

VulkanSplatting requires the following dependencies:

`Vulkan headers, Vulkan validation layers, glslangValidator, glfw, glm`

The easiest way to install the first three is through the [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).
Alternatively, you can install the corresponding packages from your distro. For Ubuntu, the packages to install
are `vulkan-headers, vulkan-validationlayers, glslang-dev, libglfw3-dev, libglm-dev`.

### Windows

After installing Vulkan SDK, set the `VULKAN_SDK` environmental variable to the install path. Alternatively,
pass `-DVULKAN_SDK=\INSTALL\LOCATION\OF\YOUR\SDK` to CMake when configuring.

A full CMake configure command is as follows:

```
mkdir .\VulkanSplatting\build
cmake -DCMAKE_BUILD_TYPE=Release -DVULKAN_SDK=\INSTALL\LOCATION\OF\YOUR\SDK -S .\VulkanSplatting -B .\VulkanSplatting\build
```

### macOS

After installing the Vulkan SDK, please proceed with CMake configuration and build steps as usual.

## TODO

- [x] Better controls and GUI on GLFW
- [ ] Implement SOTA parallel radix sort
- [ ] Use Vulkan subgroups to batch Gaussian retrievals at the warp level
- [ ] OpenXR support
- [ ] Apps for iOS and visionOS
- [ ] App for Android and Qualcomm Spaces
- [ ] Training

Please feel free to open an issue if you have any feature suggestions or are interested in contributing.

## Contributing
If you are interested in integrating your Gaussian Splatting variant, please open an issue or a pull request.
VulkanSplatting's shaders follow the procedures outlined in the original paper, so it should be relatively
easy to port your CUDA code. With cross-platform support, it's a great way to expand the reach and adoption of your research.
If there are any questions, feel free to [send me an email](mailto:me@stevengao.net).

## License

The main project is licensed under LGPL.

This project uses several third-party libraries. Here is a list of these libraries along with their respective licenses:

- **GLM**: [MIT License](https://opensource.org/licenses/MIT).
- **args.hxx**: [MIT License](https://opensource.org/licenses/MIT).
- **spdlog**: [MIT License](https://opensource.org/licenses/MIT).
- **ImGUI**: [MIT License](https://opensource.org/licenses/MIT).
- **Vulkan Memory Allocator**: [MIT License](https://opensource.org/licenses/MIT).
- **VkRadixSort**: [MIT License](https://opensource.org/licenses/MIT).
- **implot**: [MIT License](https://opensource.org/licenses/MIT).
- **glfw**: [zlib/libpng license](https://www.glfw.org/license.html).
- **libenvpp**: [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).
