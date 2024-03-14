cmake_minimum_required(VERSION 3.26)
project(vulkan_splatting_viewer)

add_executable(vulkan_splatting_viewer main.cpp)

target_include_directories(vulkan_splatting_viewer PRIVATE third_party)

target_link_libraries(vulkan_splatting_viewer PRIVATE
    libenvpp::libenvpp
    vulkan_splatting)