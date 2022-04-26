cmake_minimum_required(VERSION 3.0)

# use git version as library version
find_package(Git QUIET)
if (Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE git_version
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else ()
    set(git_version 0)
endif ()

project(OpenixCard LANGUAGES C CXX VERSION 1.0.${git_version})

add_subdirectory(src/OpenixIMG)
add_subdirectory(lib/libgenimage)
add_subdirectory(lib/inicpp)
add_subdirectory(lib/ftxui)
add_subdirectory(lib/argparse)

include_directories(
        src/OpenixIMG/include
        src/OpenixCard
        src/OpenixCard/payloads
        lib/ftxui/include
        lib/argparse/include
        lib/inicpp/include
)

file(GLOB OpenixCardSource src/OpenixCard/*.cpp)

file(GLOB OpenixCardPayloads src/OpenixCard/payloads/*.cpp)

add_executable(OpenixCard ${OpenixCardSource} ${OpenixCardPayloads})
target_link_libraries(OpenixCard OpenixIMG inicpp)