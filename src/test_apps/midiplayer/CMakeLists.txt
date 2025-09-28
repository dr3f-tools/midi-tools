cmake_minimum_required(VERSION 3.27)
project(midiplayer)

add_executable(midiplayer midiplayer.cpp)
target_link_libraries(midiplayer PRIVATE
    libremidi::libremidi
    GStreamer::GStreamer
    core::logger
)