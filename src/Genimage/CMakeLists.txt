cmake_minimum_required(VERSION 3.0)

project(Genimage)

file(GLOB GenimageSource *.c *.h)

add_library(Genimage ${GenimageSource})