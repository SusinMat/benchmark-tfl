cmake_minimum_required(VERSION 3.5)

# Project
project(benchmark_tfl C CXX)

# Set C++11 as a minimum standard for the whole project
if (CMAKE_CROSSCOMPILING)
    set(CMAKE_CXX_STANDARD 11)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include_directories(${TENSORFLOW_ROOT_DIR}
    # include TFL headers
    )

install(TARGETS benchmark_tfl EXPORT benchmark_tfl_export
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

file (GLOB LABEL_IMAGE_SOURCES "${TENSORFLOW_LITE_ROOT_DIR}/label_image/*.cc")
list(REMOVE_ITEM LABEL_IMAGE_SOURCES "${TENSORFLOW_LITE_ROOT_DIR}/label_image/label_image_test.cc")
add_executable(label_image ${LABEL_IMAGE_SOURCES})
target_link_libraries(label_image tensorflow-lite)

install(TARGETS label_image DESTINATION bin)

# Installation and packaging
# ----------------------------------------------------------------------------
# CPack stuff
# ----------------------------------------------------------------------------
include(CPackLists)

# vim: set ts=4 sts=4 sw=4 et:
