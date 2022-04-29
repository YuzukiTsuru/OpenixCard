cmake_minimum_required(VERSION 3.11)

# use git version as library version
find_package(Git QUIET)
if (Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE _git_version
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
            OUTPUT_VARIABLE PROJECT_GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
else ()
    set(_git_version 0)
    set(PROJECT_GIT_HASH "")
endif ()

project(OpenixCard LANGUAGES C CXX VERSION ${_git_version})

if (MSVC)
    message(ERROR "OpenixCard does not support MSVC")
else ()
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/dist")

    include(CMake/genimage.cmake)

    add_subdirectory(src/OpenixIMG)
    add_subdirectory(lib/inicpp)
    add_subdirectory(lib/argparse)
    
    set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(lib/ftxui)

    configure_file(
            "${PROJECT_SOURCE_DIR}/config.h.in"
            "${PROJECT_BINARY_DIR}/config.h"
    )

    include_directories(
            ${PROJECT_BINARY_DIR}
            src/OpenixIMG/include
            src/OpenixCard
            src/OpenixCard/payloads
            src/OpenixIMG/lib/rc6/src
            src/OpenixIMG/lib/twofish/src
            lib/ColorCout/includes
            lib/argparse/include
            lib/inicpp/include
            lib/cpp-subprocess
    )

    file(GLOB OpenixCardSource src/OpenixCard/*.cpp)

    file(GLOB OpenixCardPayloads src/OpenixCard/payloads/*.cpp)

    add_executable(OpenixCard ${OpenixCardSource} ${OpenixCardPayloads})
    target_link_libraries(OpenixCard OpenixIMG inicpp)

endif ()
