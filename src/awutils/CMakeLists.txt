cmake_minimum_required(VERSION 3.0)

include_directories(src)

add_subdirectory(lib/twofish)


add_library(awutils-awimg src/awimage.c src/parsecfg.c src/rc6.c)
target_link_libraries(awutils-awimg twofish)

