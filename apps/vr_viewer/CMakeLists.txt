cmake_minimum_required(VERSION 3.26)

project(3dgs_vr_viewer)

find_package(Vulkan REQUIRED)

FetchContent_Declare(OpenXR
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
        GIT_TAG release-1.0.34
)

FetchContent_GetProperties(OpenXR)
if (NOT OpenXR_POPULATED)
    FetchContent_Populate(OpenXR)
    add_subdirectory(${openxr_SOURCE_DIR} ${openxr_BINARY_DIR})
endif ()

add_executable(3dgs_vr_viewer
        src/main.cpp
        src/VRViewer.cpp
        src/VRViewer.h
        src/OXRUtils.h
        src/OXRContext.cpp
        src/OXRContext.h
)

target_compile_definitions(3dgs_cpp PUBLIC VKGS_ENABLE_OPENXR)
target_compile_definitions(3dgs_vr_viewer PRIVATE XR_USE_GRAPHICS_API_VULKAN)

target_include_directories(3dgs_vr_viewer PRIVATE
        ${openxr_SOURCE_DIR}/include
        ${spdlog_SOURCE_DIR}/include
        ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(3dgs_vr_viewer PRIVATE 3dgs_cpp openxr_loader third_party)