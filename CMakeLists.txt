cmake_minimum_required(VERSION 3.26)
project(3dgs_cpp_root)

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
FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif()

FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.3
)
FetchContent_MakeAvailable(imgui)

if (APPLE)
    add_compile_definitions(__APPLE__)

    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "/usr/local/lib")
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG)
else ()
    add_compile_definitions(NDEBUG)
endif ()

add_compile_definitions(VKGS_ENABLE_GLFW)

add_subdirectory(src)
add_subdirectory(apps)