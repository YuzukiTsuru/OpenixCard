set(CPACK_GENERATOR "DEB" "TGZ" "STGZ" "ZIP")

set(CPACK_PACKAGE_VENDOR "YuzukiTsuru")
set(CPACK_PACKAGE_DESCRIPTION "Open Source Version of Allwinner PhoenixCard to Dump, Unpack, Flash Allwinner IMG Files")

set(CPACK_DEBIAN_PACKAGE_NAME "OpenixCard")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "YuzukiTsuru")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Open Source Version of Allwinner PhoenixCard to Dump, Unpack, Flash Allwinner IMG Files")
set(CPACK_DEBIAN_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libconfuse-dev")
