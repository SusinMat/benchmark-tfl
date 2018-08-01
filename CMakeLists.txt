cmake_minimum_required(VERSION 3.5)

# Project
project(benchmark_tfl C CXX)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (NOT DEFINED TENSORFLOW_LITE_INSTALL_DIR)
    set(TENSORFLOW_LITE_INSTALL_DIR "/tmp/${user}/tfl")
endif ()

include_directories(
    "${TENSORFLOW_LITE_INSTALL_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/label_image"
    )

file(GLOB LABEL_IMAGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/label_image/*.cc")
list(REMOVE_ITEM LABEL_IMAGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/label_image/label_image_test.cc")
add_executable(label_image "${LABEL_IMAGE_SOURCES}")
target_link_libraries(label_image "${TENSORFLOW_LITE_INSTALL_DIR}/lib/libtensorflow-lite.a")

install(TARGETS label_image DESTINATION bin)
target_link_libraries(label_image "${CMAKE_DL_LIBS}")
find_package(Threads REQUIRED)
target_link_libraries(label_image Threads::Threads)

# Installation and packaging
# ----------------------------------------------------------------------------
# CPack stuff
# ----------------------------------------------------------------------------
# include(CPackLists)

# vim: set ts=4 sts=4 sw=4 et:
