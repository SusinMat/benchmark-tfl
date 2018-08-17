cmake_minimum_required(VERSION 3.6)
# Project
project(benchmark_tfl C CXX)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" CACHE PATH "Path to the toolchain that will be used to compile and link")

set(DEFAULT_BUILD_TYPE "Release")

set(TIZEN_SDK "$ENV{HOME}/tizen-studio" CACHE PATH "Location of Tizen Studio.")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified")
    set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE} CACHE STRING "Type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
else()
    set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Type of build" FORCE)
endif()

set(TENSORFLOW_LITE_SOURCE_DIR "$ENV{HOME}/tensorflow/tensorflow/contrib/lite" CACHE PATH "TensorFlow Lite source directory")

# set(TENSORFLOW_LITE_INSTALL_DIR "/tmp/$ENV{USER}/tensorflow_lite" CACHE PATH "TensorFlow Lite installation directory")
set(TENSORFLOW_LITE_INSTALL_DIR "${CMAKE_BINARY_DIR}/tensorflow_lite" CACHE PATH "TensorFlow Lite installation directory")

if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    get_filename_component(TOOLCHAIN_PATH "${CMAKE_TOOLCHAIN_FILE}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}" FORCE)
endif()

ExternalProject_Add(tensorflow_lite
    PREFIX "${TENSORFLOW_LITE_INSTALL_DIR}"
    SOURCE_DIR "${TENSORFLOW_LITE_SOURCE_DIR}"
    INSTALL_DIR "${TENSORFLOW_LITE_INSTALL_DIR}"
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX:PATH=${TENSORFLOW_LITE_INSTALL_DIR} -DCMAKE_TOOLCHAIN_FILE:PATH=${TOOLCHAIN_PATH} -DTIZEN_DEVICE=${TIZEN_DEVICE} -DTIZEN_TARGET=${TIZEN_TARGET} -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE} -DTIZEN_SDK=${TIZEN_SDK}
    PREFIX="${TENSORFLOW_LITE_INSTALL_DIR}"
    )

find_package(Threads REQUIRED)

include_directories(
    "${TENSORFLOW_LITE_INSTALL_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/beeswax"
    "${CMAKE_CURRENT_SOURCE_DIR}/bumblebee"
    )

# beeswax
file(GLOB BEESWAX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/beeswax/*.cc")
add_executable(beeswax "${BEESWAX_SOURCES}")
add_dependencies(beeswax tensorflow_lite)
target_link_libraries(beeswax "${TENSORFLOW_LITE_INSTALL_DIR}/lib/libtensorflow-lite.a")

target_link_libraries(beeswax dl)
target_link_libraries(beeswax Threads::Threads)

# bumblebee
file(GLOB BUMBLEBEE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/bumblebee/*.c")
add_executable(bumblebee "${BUMBLEBEE_SOURCES}")
target_link_libraries(bumblebee Threads::Threads)

install(TARGETS bumblebee DESTINATION bin)

# vim: set ts=4 sts=4 sw=4 et:
