#------------------------------------------------------------------------------
# Top-level CMake file for libuzmtp
#------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5.1)

# Testing properties (cmocka WITH_STATIC_LIB)
enable_testing()
include(CTest)

# Debug only builds of linux. On target hardware builds can optimize and do not use cmake
set(CMAKE_BUILD_TYPE Debug)

# TODO - enable pedantic builds on pedantic branch
#set(CMAKE_C_FLAGS "-Wall -Werror -Wpedantic -std=c11")
set(CMAKE_C_FLAGS "-Wall -Werror")

# external dependency for etst
set(WITH_STATIC_LIB ON CACHE BOOL "Build static lib" FORCE)

# other base includes includes
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include(cmake/CodeCoverage.cmake)
# TODO - only build libzmq and cmq if tests are enabled
include(cmake/cmocka.cmake)
include(cmake/czmq.cmake)
include(cmake/libzmq.cmake)

APPEND_COVERAGE_COMPILER_FLAGS()

# build libraries and their unit tests
add_subdirectory(src)

# build test apps
add_subdirectory(test/unit)
add_subdirectory(test/integration)

# Exclude test runners from the coverage report
set(COVERAGE_EXCLUDES 'test/*')
SETUP_TARGET_FOR_COVERAGE(
    NAME test_coverage 
    EXECUTABLE 
	COMMAND zmtp-test
    )
