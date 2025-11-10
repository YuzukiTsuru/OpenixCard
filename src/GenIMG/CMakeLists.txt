# Tool to build libgenimage

file(GLOB GenIMGSource *.c)

set(LIBGENIMAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/GenIMG/genimage-src")
set(LIBGENIMAGE_TARGET_BUILD_DIR "${CMAKE_BINARY_DIR}/thirdparty/genimage-src/")
set(LIBGENIMAGE_TARGET_LIBRARY "${CMAKE_BINARY_DIR}/thirdparty/genimage-src/libgenimage.a")

# Find libconfuse
find_package(PkgConfig REQUIRED)
pkg_check_modules(CONFUSE REQUIRED libconfuse)

add_library(GenIMG ${GenIMGSource} GenIMG.cpp GenIMG.h GenimageWrapper.h)
target_include_directories(GenIMG PRIVATE ${CONFUSE_INCLUDE_DIRS})
target_link_libraries(GenIMG PRIVATE ${LIBGENIMAGE_TARGET_LIBRARY} Threads::Threads ${CONFUSE_LIBRARIES})
target_compile_options(GenIMG PRIVATE ${CONFUSE_CFLAGS_OTHER})

file(MAKE_DIRECTORY ${LIBGENIMAGE_TARGET_BUILD_DIR})

# Copy genimage-src files to build directory using CMake for cross-platform compatibility
add_custom_command(
        TARGET GenIMG
        PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIBGENIMAGE_SOURCE_DIR}" "${LIBGENIMAGE_TARGET_BUILD_DIR}"
)

# autotools build
add_custom_command(
        TARGET GenIMG
        PRE_LINK
        WORKING_DIRECTORY ${LIBGENIMAGE_TARGET_BUILD_DIR}
        COMMAND aclocal
)

add_custom_command(
        TARGET GenIMG
        PRE_LINK
        WORKING_DIRECTORY ${LIBGENIMAGE_TARGET_BUILD_DIR}
        COMMAND autoconf -i && automake --add-missing
)

add_custom_command(
        TARGET GenIMG
        PRE_LINK
        WORKING_DIRECTORY ${LIBGENIMAGE_TARGET_BUILD_DIR}
        COMMAND chmod a+x configure && ./configure && make
)
