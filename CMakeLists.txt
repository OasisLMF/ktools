cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME ktools)

project(${PROJECT_NAME})

# Create the VERSION define
file(READ "${CMAKE_SOURCE_DIR}/VERSION.in" KTOOLS_VERSION)
string(REGEX MATCH "([0-9]\.[0-9]\.[0-9])" _ ${KTOOLS_VERSION})
set(KTOOLS_VERSION ${CMAKE_MATCH_1})

# Should not be done, but it is to maintain compatibility with the autotools build
configure_file(config.h.cmake_in ${CMAKE_SOURCE_DIR}/config.h)

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
	add_compile_definitions( _CRT_SECURE_NO_WARNINGS)
    add_compile_options( /wd4503 ) # truncated symbol
    add_compile_options( /wd4702 ) # unreachable code
    add_compile_options( /wd4091 ) # typedef ignored on left when no variable is declared
    add_compile_options( /bigobj )
    add_definitions( /DUNICODE /D_UNICODE ) # it is a new millenium
endif()


add_subdirectory(src)

file(COPY ${CMAKE_SOURCE_DIR}/ktest DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/examples DESTINATION ${CMAKE_BINARY_DIR})

include(CTest) 
add_test(ktest ${CMAKE_BINARY_DIR}/ktest/runtests.sh WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ktest)
add_test(ktest-extended ${CMAKE_BINARY_DIR}/ktest/runtestsx.sh WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ktest)


