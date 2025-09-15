include(FetchContent)

find_package(ALSA REQUIRED)
find_package(Boost REQUIRED)

FetchContent_Declare(
    readerwriterqueue
    GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue.git
    GIT_TAG        v1.0.7
)
FetchContent_MakeAvailable(readerwriterqueue)

FetchContent_Declare(
    libremidi
    GIT_REPOSITORY https://github.com/celtera/libremidi
    GIT_TAG        v5.3.1
)
FetchContent_MakeAvailable(libremidi)
target_compile_options(libremidi PRIVATE -Wno-maybe-uninitialized) # silence a Boost warning from libremidi

find_package(GStreamer REQUIRED)