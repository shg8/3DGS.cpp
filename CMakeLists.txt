cmake_minimum_required(VERSION 3.26)
project(vulkan_splatting)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan COMPONENTS glslc)

find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

FetchContent_Declare(libenvpp
        GIT_REPOSITORY https://github.com/ph3at/libenvpp.git
        GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(libenvpp)

FetchContent_Declare(fuchsia_radix_sort GIT_REPOSITORY https://github.com/juliusikkala/fuchsia_radix_sort.git GIT_TAG main)
FetchContent_MakeAvailable(fuchsia_radix_sort)

if (APPLE)
    add_compile_definitions(__APPLE__)
endif ()

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "shaders/*.frag"
        "shaders/*.vert"
        "shaders/*.comp"
)

if (CMAKE_BUILD_TYPE MATCHES Debug AND NOT APPLE)
    set(GLSLC_DEFINE "-DDEBUG")
else ()
    set(GLSLC_DEFINE "-DNDEBUG")
endif ()

set(GLSLC_DEFINE "${GLSLC_DEFINE}")

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND ${glslc_executable} ${GLSL} -o ${SPIRV} ${GLSLC_DEFINE} "--target-env=vulkan1.2"
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

include_directories(third_party)

add_executable(vulkan_splatting main.cpp
        vulkan/VulkanContext.cpp
        3dgs/Renderer.cpp
        vulkan/Window.cpp
        3dgs/GSScene.cpp
        vulkan/Shader.cpp
        vulkan/Utils.cpp
        vulkan/VMA.cpp
        vulkan/Buffer.cpp
        vulkan/Buffer.h
        vulkan/pipelines/Pipeline.cpp
        vulkan/pipelines/Pipeline.h
        vulkan/DescriptorSet.cpp
        vulkan/DescriptorSet.h
        vulkan/pipelines/ComputePipeline.cpp
        vulkan/pipelines/ComputePipeline.h
        vulkan/Swapchain.cpp
        vulkan/Swapchain.h
)

add_dependencies(vulkan_splatting Shaders)

target_include_directories(vulkan_splatting PUBLIC ${Vulkan_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${VK_RADIX_SORT_INCLUDE_DIRS})

target_link_libraries(vulkan_splatting PUBLIC glfw libenvpp::libenvpp vk-radix-sort)
target_link_libraries(vulkan_splatting PUBLIC Vulkan::Vulkan)

add_custom_command(TARGET vulkan_splatting POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:vulkan_splatting>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:vulkan_splatting>/shaders"
)