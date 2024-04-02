cmake_minimum_required(VERSION 3.26)

add_library(third_party INTERFACE)

target_include_directories(third_party INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)