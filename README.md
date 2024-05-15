# 3DGS.cpp

3DGS.cpp is a high performance, cross-platform implementation
of [Gaussian Splatting](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/) using
the [Vulkan API](https://www.khronos.org/vulkan/) and compute pipelines.

**Why Vulkan?** We want to democratize the access to high-performance point-based radiance fields.
Existing implementations of Gaussian Splatting are often limited to CUDA, which only runs on NVIDIA GPUs, 
and OpenGL is deprecated on Apple platforms. Additionally, Vulkan's compute capabilities are the closest to CUDA's 
with support for warp-level primitives (subgroups).

[![Windows + Linux](https://github.com/shg8/3DGS.cpp/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=main)](https://github.com/shg8/3DGS.cpp/actions/workflows/cmake-multi-platform.yml)

![3DGS.cpp Demo macOS](https://github.com/shg8/3DGS.cpp/assets/38004233/66542056-ce30-4998-a612-dd4f6792599e)

## Downloads
* [Windows](https://github.com/shg8/3DGS.cpp/releases/download/nightly/3dgs_cpp_viewer-windows-latest-amd64-nightly)
* [macOS (arm64)](https://github.com/shg8/3DGS.cpp/releases/download/nightly/3dgs_cpp_viewer-macos-14-arm64-nightly)
* [Linux](https://github.com/shg8/3DGS.cpp/releases/download/nightly/3dgs_cpp_viewer-ubuntu-latest-amd64-nightly)
* iOS, iPad OS, visionOS (as an iPad app) on [TestFlight](https://testflight.apple.com/join/0WzabeP7)

## Command Line Viewer

```
  ./3dgs_cpp_viewer {OPTIONS} [scene]

    Vulkan Splatting

  OPTIONS:

      -h, --help                        Display this help menu
      --validation                      Enable Vulkan validation layers
      -v, --verbose                     Enable verbose logging
      -d[physical-device],
      --device=[physical-device]        Select physical device by index
      -i, --immediate-swapchain         Set swapchain mode to immediate
                                        (VK_PRESENT_MODE_IMMEDIATE_KHR)
      -w[width], --width=[width]        Set window width
      -h[height], --height=[height]     Set window height
      --no-gui                          Disable GUI
      scene                             Path to scene fil
```

## Building
### Linux

3DGS.cpp requires the following dependencies:

`Vulkan headers, Vulkan validation layers, glslangValidator, glfw, glm`

The easiest way to install the first three is through the [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).
Alternatively, you can install the corresponding packages from your distro. For Ubuntu, the packages to install
are `vulkan-headers, vulkan-validationlayers, glslang-dev, libglfw3-dev, libglm-dev`.

```
git clone https://github.com/shg8/3DGS.cpp/
cd 3DGS.cpp
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -S ./ -B ./build
cmake --build ./build -j4
```
The command line viewer will be under `build/apps/viewer/`.
### Windows

After installing Vulkan SDK, set the `VULKAN_SDK` environmental variable to the install path. Alternatively,
pass `-DVULKAN_SDK=\INSTALL\LOCATION\OF\YOUR\SDK` to CMake when configuring.

### macOS
After installing the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/), please proceed with CMake configuration and build steps.

### iOS, iPadOS, visionOS
1. Make sure that the Vulkan SDK is installed
2. Run the `xcode_setup` CMake target
3. Download the [prebuilt MoltenVK libraries](https://github.com/KhronosGroup/MoltenVK/actions) or build it yourself. Unzip the downloaded artifact from a MoltenVK workflow. If you're not sure which one you want to use, pick the one from the latest tag
4. Place the MoltenVK folder that contains the `dynamic`, `include`, and `static` folders under `apps/apple`
5. Set your development team id in `project.xcconfig`
6. Profit

## TODO

- [x] Better controls and GUI on GLFW
- [x] Apps for iOS and visionOS
- [ ] Fully immersive app on visionOS using the Compositor Service framework
- [ ] OpenXR support
- [ ] App for Android and Qualcomm Spaces
- [ ] Implement SOTA parallel radix sort
- [ ] Use Vulkan subgroups to batch Gaussian retrievals at the warp level
- [ ] Training

Please feel free to open an issue if you have any feature suggestions or are interested in contributing.

## Benchmarking
There is room for performance improvements. See benchmarks on a 4090.
![cumulative_duration](https://github.com/shg8/3DGS.cpp/assets/38004233/91e6a082-95ec-430d-bbbb-cbb3f63795e0)

## Contributing
If you are interested in integrating your Gaussian Splatting variant, please open an issue or a pull request.
3DGS.cpp's shaders follow the procedures outlined in the original paper, so it should be relatively
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
