# Tool to build libgenimage

file(GLOB GenimageWrapperSource *.c)

include_directories(
        ${CMAKE_SOURCE_DIR}/src/Genimage/genimage-src
)

add_library(GenimageWrapper ${GenimageWrapperSource})

set(LIBGENIMAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/Genimage/genimage-src")
set(LIBGENIMAGE_TARGET_BUILD_DIR "${CMAKE_BINARY_DIR}/thirdparty/genimage-src/")

file(MAKE_DIRECTORY ${LIBGENIMAGE_TARGET_BUILD_DIR})

add_custom_command(
        TARGET GenimageWrapper
        PRE_BUILD
        COMMAND cp -rf "${LIBGENIMAGE_SOURCE_DIR}/*" "${LIBGENIMAGE_TARGET_BUILD_DIR}"
)

# autoconf -i
add_custom_command(
        TARGET GenimageWrapper
        PRE_BUILD
        WORKING_DIRECTORY ${LIBGENIMAGE_TARGET_BUILD_DIR}
        COMMAND automake && autoconf -i && ./configure && make
)
