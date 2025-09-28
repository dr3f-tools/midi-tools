cmake_minimum_required(VERSION 3.27)
project(midi-tools)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(build)
include(deps)

add_subdirectory(3rdparty)

add_subdirectory(src/modules/logger)

add_subdirectory(src/test_apps/gstreamer)
add_subdirectory(src/test_apps/libremidi)