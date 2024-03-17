cmake_minimum_required(VERSION 3.26)
project(vulkan_splatting)

add_subdirectory(shaders)

file(GLOB EXTERNAL_SOURCE
        ${imgui_SOURCE_DIR}/*.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        third_party/implot/implot.cpp
        third_party/implot/implot_demo.cpp
        third_party/implot/implot_items.cpp
)

file(GLOB SOURCE
        *.cpp
        vulkan/*.cpp
        vulkan/pipelines/*.cpp
        vulkan/windowing/GLFWWindow.cpp)

add_library(vulkan_splatting STATIC
        ${SOURCE}
        ${EXTERNAL_SOURCE}
)

target_include_directories(vulkan_splatting
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include/vulkan_splatting
        PRIVATE
        third_party
        ${Vulkan_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
        ${glm_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${spdlog_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/shaders
)

add_dependencies(vulkan_splatting shaders)

target_link_libraries(vulkan_splatting PUBLIC Vulkan::Vulkan glfw spdlog::spdlog)
if (UNIX)
    target_link_libraries(vulkan_splatting PUBLIC ${CMAKE_DL_LIBS})
endif ()