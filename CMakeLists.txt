cmake_minimum_required(VERSION 3.27)
project(midi-tools)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(deps)

add_executable(midireader src/midireader.cpp)
target_link_libraries(midireader PRIVATE
    libremidi::libremidi
)

add_executable(gstreamer src/gstreamer.cpp)
target_link_libraries(gstreamer PRIVATE
    GStreamer::GStreamer
)