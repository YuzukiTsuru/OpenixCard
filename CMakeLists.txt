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

project(OpenixCard LANGUAGES CXX VERSION 1.0.${git_version})

add_subdirectory(src/AllwinnerIMGUtils)
add_subdirectory(lib/ftxui)
add_subdirectory(lib/argparse)

include_directories(
        src/OpenixCard
        lib/ftxui/include
        lib/argparse/include
)

file(GLOB OpenixCardSource src/OpenixCard/*.cpp)

add_executable(OpenixCard ${OpenixCardSource})
target_link_libraries(OpenixCard awutils-awimg)