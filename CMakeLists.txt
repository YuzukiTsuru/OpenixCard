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

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
    FetchContent_Populate(ftxui)
    add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include(CMake/genimage.cmake)

add_subdirectory(src/OpenixIMG)
add_subdirectory(lib/inicpp)
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