cmake_minimum_required(VERSION 3.0)

project(OpenixIMG)

include_directories(
        src
        lib/twofish/src
        lib/rc6/src
)

add_subdirectory(lib/twofish)
add_subdirectory(lib/rc6)

add_library(OpenixIMG src/OpenixIMG.c src/CFGParser.c)
target_link_libraries(OpenixIMG twofish)

add_executable(T_OpenixIMG test/T_OpenixIMG.c)
target_link_libraries(T_OpenixIMG OpenixIMG)