cmake_minimum_required(VERSION 3.6)

# Project
project(benchmark_tfl C CXX)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(TENSORFLOW_LITE_INSTALL_DIR "/tmp/$ENV{USER}/tensorflow_lite" CACHE PATH "TensorFlow Lite installation directory")

find_package(Threads REQUIRED)

include_directories(
    "${TENSORFLOW_LITE_INSTALL_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/label_image"
    "${CMAKE_CURRENT_SOURCE_DIR}/beeswax"
    )

# label_image
file(GLOB LABEL_IMAGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/label_image/*.cc")
add_executable(label_image "${LABEL_IMAGE_SOURCES}")
target_link_libraries(label_image "${TENSORFLOW_LITE_INSTALL_DIR}/lib/libtensorflow-lite.a")

install(TARGETS label_image DESTINATION bin)
target_link_libraries(label_image dl)
target_link_libraries(label_image Threads::Threads)

# beeswax
file(GLOB LABEL_IMAGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/beeswax/*.cc")
add_executable(beeswax "${LABEL_IMAGE_SOURCES}")
target_link_libraries(beeswax "${TENSORFLOW_LITE_INSTALL_DIR}/lib/libtensorflow-lite.a")

install(TARGETS beeswax DESTINATION bin)
target_link_libraries(beeswax dl)
target_link_libraries(beeswax Threads::Threads)



# vim: set ts=4 sts=4 sw=4 et:
