cmake_minimum_required(VERSION 3.26)

project(3dgs_viewer)

FetchContent_Declare(libenvpp
        GIT_REPOSITORY https://github.com/ph3at/libenvpp.git
        GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(libenvpp)

add_executable(3dgs_viewer src/main.cpp
)

target_compile_definitions(3dgs_cpp PUBLIC
        VKGS_ENABLE_GLFW
)

target_link_libraries(3dgs_viewer PRIVATE
        libenvpp::libenvpp
        3dgs_cpp
        third_party
)
