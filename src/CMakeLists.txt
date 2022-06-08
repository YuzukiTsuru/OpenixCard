add_subdirectory(GenIMG)
add_subdirectory(OpenixIMG)
add_subdirectory(OpenixCard)

add_executable(OpenixCard main.cpp)
target_link_libraries(OpenixCard PRIVATE libOpenixCard OpenixIMG inicpp GenIMG)