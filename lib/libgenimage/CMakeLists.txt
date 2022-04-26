cmake_minimum_required(VERSION 3.0)

project(genimage)

include_directories(
        genimage/
)

file(GLOB genimage_src genimage/*.c)
add_library(genimage ${genimage_src})