cmake_minimum_required(VERSION 3.0)

project(OpenixIMG)

include_directories(
        include
        lib/twofish/src
        lib/rc6/src
)

add_subdirectory(lib/twofish)
add_subdirectory(lib/rc6)

add_library(OpenixIMG src/OpenixIMG.c ../GenIMG/GenimageWrapper.c)
target_link_libraries(OpenixIMG twofish rc6)

option(BUILD_T_OpenixIMG "Set to ON to build OpenixIMG Test" OFF)

if(BUILD_T_OpenixIMG)

add_executable(T_OpenixIMG test/T_OpenixIMG.c)
target_link_libraries(T_OpenixIMG OpenixIMG)

endif()
