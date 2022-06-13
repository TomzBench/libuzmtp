#------------------------------------------------------------------------------
# Top-level CMake file for libuzmtp
#------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
project(libuzmtp)

#------------------------------------------------------------------------------
# Helpful SETS
#------------------------------------------------------------------------------
set(FIXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test/integration/fixture")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(DOWNLOAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dl")
set(CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(CMAKE_EXTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/cmake/external")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT EXISTS EXTERNAL_DIR)
    FILE(MAKE_DIRECTORY ${EXTERNAL_DIR})
endif()

#------------------------------------------------------------------------------
# helper modules
#------------------------------------------------------------------------------
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include(${CMAKE_MODULES_DIR}/MakeAbsolute.cmake)
include(${CMAKE_MODULES_DIR}/ExtractChecked.cmake)
include(${CMAKE_DIR}/supported_platforms.cmake)

#------------------------------------------------------------------------------
# Package options
#------------------------------------------------------------------------------
option(UZMTP_ENABLE_TESTING "build libuzmtp tests" ON)
set(UZMTP_PLATFORM "LINUX" CACHE STRING "Host operating system [${SUPPORTED_PLATFORMS}]")

if(UZMTP_PLATFORM STREQUAL ZEPHYR)
    include(${CMAKE_DIR}/build_zephyr.cmake)
elseif(UZMTP_PLATFORM STREQUAL LINUX)
    include(${CMAKE_DIR}/build_linux.cmake)
endif()
