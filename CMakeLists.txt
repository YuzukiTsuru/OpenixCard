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

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    message(ERROR "OpenixCard does not support MSVC")
else ()
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/dist")

    find_package(Threads REQUIRED)

    set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(INICPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(INICPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

    add_subdirectory(lib/inicpp EXCLUDE_FROM_ALL)
    add_subdirectory(lib/argparse EXCLUDE_FROM_ALL)
    add_subdirectory(lib/ftxui EXCLUDE_FROM_ALL)

    configure_file(
            "${PROJECT_SOURCE_DIR}/config.h.in"
            "${PROJECT_BINARY_DIR}/config.h"
    )

    include_directories(
            ${PROJECT_BINARY_DIR}
            src/OpenixIMG/include
            src/OpenixCard
            src/GenIMG
            src/OpenixCard/payloads
            src/GenIMG/genimage-src
            src/OpenixIMG/lib/rc6/src
            src/OpenixIMG/lib/twofish/src
            lib/ColorCout/includes
            lib/argparse/include
            lib/inicpp/include
            lib/cpp-subprocess
            lib/ftxui/include
    )

    add_subdirectory(src)

    install(
            TARGETS OpenixCard
            EXPORT OpenixCard
            COMPONENT applications
            RUNTIME DESTINATION bin
    )

    include(CMake/cpack.cmake)
    include(CPack)
endif ()
