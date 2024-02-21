cmake_minimum_required(VERSION 3.26)
project(vulkan_splatting)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan COMPONENTS glslangValidator)

if (WIN32 OR APPLE)
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw
            GIT_TAG 3.3.9
    )

    FetchContent_GetProperties(glfw)
    if (NOT glfw_POPULATED)
        FetchContent_Populate(glfw)

        set(GLFW_INSTALL OFF CACHE INTERNAL "Create GLFW installation target")
        set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build GLFW documentation")
        set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build GLFW test programs")
        set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build GLFW examples")

        add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
    endif()

    set(GLM_ENABLE_CXX_20 ON CACHE INTERNAL "Enable experimental features")

    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm
            GIT_TAG 1.0.0
    )
    FetchContent_MakeAvailable(glm)

    if (MSVC)
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    endif ()
else ()
    find_package(glfw3 3.3 REQUIRED)
    find_package(glm REQUIRED)
endif ()


FetchContent_Declare(libenvpp
        GIT_REPOSITORY https://github.com/ph3at/libenvpp.git
        GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(libenvpp)

FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.3
)
FetchContent_MakeAvailable(imgui)

if (APPLE)
    add_compile_definitions(__APPLE__)
endif ()

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "shaders/*.frag"
        "shaders/*.vert"
        "shaders/*.comp"
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG)
else ()
    add_compile_definitions(NDEBUG)
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug AND NOT APPLE)
    list(APPEND GLSLC_DEFINE "-DDEBUG")
else ()
    list(APPEND GLSLC_DEFINE "-DNDEBUG")
endif ()

if (APPLE)
    # append -DAPPLE to GLSLC_DEFINE
    list(APPEND GLSLC_DEFINE "-DAPPLE")
endif ()

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} "--target-env" "vulkan1.2" -V ${GLSL} -o ${SPIRV} ${GLSLC_DEFINE}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

include_directories(third_party)

file(GLOB EXTERNAL_SOURCE
        ${imgui_SOURCE_DIR}/*.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        third_party/implot/implot.cpp
        third_party/implot/implot_demo.cpp
        third_party/implot/implot_items.cpp
)

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
        ${EXTERNAL_SOURCE}
        vulkan/ImguiManager.cpp
        vulkan/ImguiManager.h
        vulkan/QueryManager.cpp
        vulkan/QueryManager.h
        3dgs/GUIManager.cpp
        3dgs/GUIManager.h
)

add_dependencies(vulkan_splatting Shaders)

target_include_directories(vulkan_splatting PUBLIC
        ${Vulkan_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
        ${glm_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${spdlog_SOURCE_DIR}/include
)

target_link_libraries(vulkan_splatting PUBLIC glfw libenvpp::libenvpp)
target_link_libraries(vulkan_splatting PUBLIC Vulkan::Vulkan)
if (UNIX)
    target_link_libraries(vulkan_splatting PUBLIC ${CMAKE_DL_LIBS})
endif ()

add_custom_command(TARGET vulkan_splatting POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:vulkan_splatting>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:vulkan_splatting>/shaders"
)