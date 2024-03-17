cmake_minimum_required(VERSION 3.26)
project(shaders)

add_executable(embedfile ${CMAKE_SOURCE_DIR}/cmake/embedfile.c)

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "*.frag"
        "*.vert"
        "*.comp"
)

if (CMAKE_BUILD_TYPE MATCHES Debug AND NOT APPLE)
    list(APPEND GLSLC_DEFINE "-DDEBUG")
else ()
    list(APPEND GLSLC_DEFINE "-DNDEBUG")
endif ()

if (APPLE)
    # append -DAPPLE to GLSLC_DEFINE
    list(APPEND GLSLC_DEFINE "-DAPPLE")
endif ()

set(SHADER_HEADER "${CMAKE_BINARY_DIR}/shaders/shaders.h")
set(XCODE_SHADER "${CMAKE_SOURCE_DIR}/apps/apple/VulkanSplatting/shaders.h")

message(STATUS "Shader header file: ${SHADER_HEADER}")

# Delete old header file
add_custom_command(
        OUTPUT ${SHADER_HEADER}
        COMMAND ${CMAKE_COMMAND} -E remove ${SHADER_HEADER}
        DEPENDS ${GLSL_SOURCE_FILES}
)

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME_WE)
    string(TOUPPER ${FILE_NAME} FILE_NAME_UPPER)
    set(FILE_NAME_UPPER "SPV_${FILE_NAME_UPPER}")
    set(SPIRV "${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    SET(TEMP_HEADER "${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}.h")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders/"
            COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} "--target-env" "vulkan1.2" -V ${GLSL} -o ${SPIRV} ${GLSLC_DEFINE}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})

    add_custom_command(
            OUTPUT ${SHADER_HEADER}
            COMMAND embedfile ${FILE_NAME_UPPER} ${SPIRV} ${SHADER_HEADER}
            DEPENDS ${SPIRV}
            APPEND)

    list(APPEND TEMP_HEADERS ${TEMP_HEADER})
endforeach (GLSL)

add_custom_target(shaders DEPENDS ${SHADER_HEADER} ${SPIRV_BINARY_FILES})

add_custom_target(xcode_shaders
        DEPENDS
        ${XCODE_SHADER}
)

add_custom_command(
        OUTPUT ${XCODE_SHADER}
        COMMAND ${CMAKE_COMMAND} -E copy ${SHADER_HEADER} ${XCODE_SHADER}
        DEPENDS ${SHADER_HEADER}
)