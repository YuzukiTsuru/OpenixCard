# Tool to build libgenimage

file(GLOB GenimageWrapperSource *.c)

add_library(GenimageWrapper ${GenimageWrapperSource})

set(LIBGENIMAGE_TARGET_BUILD_DIR ${CMAKE_BINARY_DIR}/thirdparty/genimage)

add_custom_command(
        TARGET GenimageWrapper
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/test/vaiueo2d.wav"
        ${LIBGENIMAGE_TARGET_BUILD_DIR}
)