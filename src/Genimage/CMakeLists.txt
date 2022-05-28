# Tool to build libgenimage

file(GLOB GenimageWrapperSource *.c)

set(LIBGENIMAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/Genimage/genimage-src")
set(LIBGENIMAGE_TARGET_BUILD_DIR "${CMAKE_BINARY_DIR}/thirdparty/genimage-src/")
set(LIBGENIMAGE_TARGET_LIBRARY "${CMAKE_BINARY_DIR}/thirdparty/genimage-src/libgenimage.a")

include_directories(
        ${LIBGENIMAGE_SOURCE_DIR}
)

add_executable(GenimageWrapper ${GenimageWrapperSource})
target_link_libraries(GenimageWrapper ${LIBGENIMAGE_TARGET_LIBRARY} confuse)

file(MAKE_DIRECTORY ${LIBGENIMAGE_TARGET_BUILD_DIR})

add_custom_command(
        TARGET GenimageWrapper
        PRE_LINK
        COMMAND cp -rf "${LIBGENIMAGE_SOURCE_DIR}/*" "${LIBGENIMAGE_TARGET_BUILD_DIR}"
)

# autoconf -i
add_custom_command(
        TARGET GenimageWrapper
        PRE_LINK
        WORKING_DIRECTORY ${LIBGENIMAGE_TARGET_BUILD_DIR}
        COMMAND automake && autoconf -i && ./configure && make
)
