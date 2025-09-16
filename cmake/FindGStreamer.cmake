# FindGStreamer.cmake
#
# Provides an imported target: GStreamer::GStreamer

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-app-1.0
    gstreamer-audio-1.0
    gstreamer-base-1.0
)

add_library(GStreamer::GStreamer INTERFACE IMPORTED)

target_include_directories(GStreamer::GStreamer INTERFACE ${GST_INCLUDE_DIRS})
target_link_libraries(GStreamer::GStreamer INTERFACE ${GST_LIBRARIES})
target_compile_options(GStreamer::GStreamer INTERFACE ${GST_CFLAGS_OTHER})
