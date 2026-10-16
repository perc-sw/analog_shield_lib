# CMake project for the AnalogShield library. The Arduino IDE / arduino-cli
# ignore this file; it exists for IDE code navigation and PC-side unit tests.
#
#   cmake --workflow --preset host-tests      build the library against a fake
#                                             Arduino API (tests/fake_arduino)
#                                             and run tests/test_*.cpp
cmake_minimum_required(VERSION 3.21)

# Version comes from library.properties, the single source of truth
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/library.properties" _version_line REGEX "^version=")
string(REGEX REPLACE "^version=" "" ANALOG_SHIELD_VERSION "${_version_line}")

project(AnalogShield VERSION ${ANALOG_SHIELD_VERSION} LANGUAGES CXX)

option(ANALOG_SHIELD_HOST_TESTS "Build the PC-side unit tests" OFF)
option(ANALOG_SHIELD_WERROR "Treat warnings as errors" OFF)

# --- Library ----------------------------------------------------------------------------
# Needs an Arduino.h on the include path: the fake one for host tests, or a
# core's when another project adds this directory.
file(GLOB_RECURSE ANALOG_SHIELD_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(analog_shield STATIC ${ANALOG_SHIELD_SOURCES})
add_library(AnalogShield::analog_shield ALIAS analog_shield)
target_include_directories(analog_shield PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(analog_shield PUBLIC cxx_std_17)
target_compile_options(analog_shield PRIVATE -Wall -Wextra $<$<BOOL:${ANALOG_SHIELD_WERROR}>:-Werror>)

# --- Tests ------------------------------------------------------------------------------
if(ANALOG_SHIELD_HOST_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
