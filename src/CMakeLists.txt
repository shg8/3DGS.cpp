cmake_minimum_required(VERSION 3.26)
project(3dgs_cpp)

# Dependencies
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

add_subdirectory(shaders)
# End Dependencies

# Source files
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
        vulkan/targets/GLFWWindow.cpp)

list(REMOVE_ITEM SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/DummyGUIManager.cpp)
# End Source files

add_library(3dgs_cpp STATIC
        ${SOURCE}
        ${EXTERNAL_SOURCE}
        vulkan/targets/OpenXRStereo.cpp
        vulkan/targets/OpenXRStereo.h
)

target_include_directories(3dgs_cpp
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include/3dgs
        PRIVATE
        third_party
        ${Vulkan_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
        ${glm_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${CMAKE_BINARY_DIR}/shaders
        ${spdlog_SOURCE_DIR}/include
)

add_dependencies(3dgs_cpp shaders)

target_link_libraries(3dgs_cpp PUBLIC Vulkan::Vulkan glfw spdlog::spdlog)
if (UNIX)
    target_link_libraries(3dgs_cpp PUBLIC ${CMAKE_DL_LIBS})
endif ()