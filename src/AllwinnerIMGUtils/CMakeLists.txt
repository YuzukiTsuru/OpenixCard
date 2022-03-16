cmake_minimum_required(VERSION 3.0)

project(awutils)

include_directories(
        src
        lib/twofish/src
        )

add_subdirectory(lib/twofish)

add_library(awutils-awimg src/awimage.c src/parsecfg.c src/rc6.c)
target_link_libraries(awutils-awimg twofish)

add_executable(awimg test/awimage_test.c)
target_link_libraries(awimg awutils-awimg)