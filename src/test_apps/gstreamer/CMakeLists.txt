cmake_minimum_required(VERSION 3.27)
project(gstreamer_apps)

add_executable(audiotestsrc audiotestsrc.cpp)
target_link_libraries(audiotestsrc PRIVATE
    GStreamer::GStreamer
)

add_executable(app_src app_src.cpp)
target_link_libraries(app_src PRIVATE
    GStreamer::GStreamer
    logger
)