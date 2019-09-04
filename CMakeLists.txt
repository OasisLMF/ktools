cmake_minimum_required(VERSION 3.13)
project(ktools)

# Create the VERSION define
set(KTOOLS_VERSION "3.1.0")
configure_file(config.h.cmake_in config.h)

ENABLE_TESTING()

# Turn on C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)

# Turn on all warnings, for all compiler flavours
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    MESSAGE( STATUS "using clang settings" )
    add_compile_options( -Wall -Wextra -Wunused-parameter)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    MESSAGE( STATUS "using gnu settings" )
    add_compile_options( -Wall -Wextra -Wunused-parameter)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    MESSAGE( STATUS "using msvc settings" )
    add_compile_options( /W4 /WX )
    add_compile_options( /wd4503 ) # truncated symbol
    add_compile_options( /wd4702 ) # unreachable code
    add_compile_options( /wd4091 ) # typedef ignored on left when no variable is declared
    add_compile_options( /bigobj )
    add_definitions( /DUNICODE /D_UNICODE ) # it is a new millenium
endif()

add_subdirectory(src)
