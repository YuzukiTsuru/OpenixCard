# Tool to build libgenimage

file(GLOB GenimageWrapperSource *.c)

add_library(GenimageWrapper ${GenimageWrapperSource})

set(LIBGENIMAGE_TARGET_BUILD_DIR ${CMAKE_BINARY_DIR}/thirdparty/genimage)

# Copy genimage files to build dir
add_custom_command(
        TARGET GenimageWrapper
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/src/Genimage/genimage-src/*"
        "${LIBGENIMAGE_TARGET_BUILD_DIR}"
)

# autoconf -i
add_custom_command(
        TARGET GenimageWrapper
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/src/Genimage/genimage-src/*"
        "${LIBGENIMAGE_TARGET_BUILD_DIR}"
)