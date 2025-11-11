add_subdirectory(GenIMG)
add_subdirectory(OpenixIMG)
add_subdirectory(OpenixCard)

# Find libconfuse for linking
find_package(PkgConfig REQUIRED)
pkg_check_modules(CONFUSE REQUIRED libconfuse)

# Main app
add_executable(OpenixCard main.cpp)
target_link_libraries(OpenixCard PRIVATE libOpenixCard OpenixIMG inicpp GenIMG ${CONFUSE_LIBRARIES})
target_link_directories(OpenixCard PRIVATE ${CONFUSE_LIBRARY_DIRS})