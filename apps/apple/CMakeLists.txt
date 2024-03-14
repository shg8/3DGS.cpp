cmake_minimum_required(VERSION 3.26)

set(XCODE_THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/apps/apple/VulkanSplatting/ThirdParty)

add_custom_target(xcode_thirdparty DEPENDS ${XCODE_THIRDPARTY_DIR})

add_custom_command(OUTPUT ${XCODE_THIRDPARTY_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${XCODE_THIRDPARTY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${glm_SOURCE_DIR} ${XCODE_THIRDPARTY_DIR}/glm
)

add_custom_target(xcode_setup DEPENDS xcode_thirdparty xcode_shaders)