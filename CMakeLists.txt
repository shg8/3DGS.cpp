cmake_minimum_required(VERSION 3.26)
project(3dgs_cpp_root)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

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

add_subdirectory(src)
add_subdirectory(apps)