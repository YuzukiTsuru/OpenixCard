include(ExternalProject)

set(GENIMAGE_ROOT          ${CMAKE_BINARY_DIR}/thirdparty/genimage)
set(GENIMAGE_LIB_DIR       ${GENIMAGE_ROOT})
set(GENIMAGE_INCLUDE_DIR   ${GENIMAGE_ROOT})

set(GENIMAGE_URL           https://github.com/pengutronix/genimage/releases/download/v15/genimage-15.tar.xz)
set(GENIMAGE_CONFIGURE     cd ${GENIMAGE_ROOT}/src/genimage/ && ./configure)
set(GENIMAGE_MAKE          cd ${GENIMAGE_ROOT}/src/genimage/ && make)
set(GENIMAGE_INSTALL       cd ${GENIMAGE_ROOT}/src/genimage/ && cp -rf genimage ${CMAKE_BINARY_DIR})

ExternalProject_Add(genimage
        URL                   ${GENIMAGE_URL}
        DOWNLOAD_NAME         genimage-15.tar.xz
        PREFIX                ${GENIMAGE_ROOT}
        CONFIGURE_COMMAND     ${GENIMAGE_CONFIGURE}
        BUILD_COMMAND         ${GENIMAGE_MAKE}
        INSTALL_COMMAND       ${GENIMAGE_INSTALL}
        )