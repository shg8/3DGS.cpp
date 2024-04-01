cmake_minimum_required(VERSION 3.26)
project(3dgs_viewer)

add_executable(3dgs_viewer main.cpp)

target_include_directories(3dgs_viewer PRIVATE third_party)

target_link_libraries(3dgs_viewer PRIVATE
    libenvpp::libenvpp
    3dgs_cpp)