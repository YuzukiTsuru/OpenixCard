cmake_minimum_required(VERSION 3.5)

project(twofish)

include_directories(src)

add_library(twofish src/twofish.c)