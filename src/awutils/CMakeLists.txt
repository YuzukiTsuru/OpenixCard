cmake_minimum_required(VERSION 3.0)

include_directories(src)
add_library(awutils-awimg src/awimage.c src/parsecfg.c src/rc6.c src/twofish.c)

