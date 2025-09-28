project(3rdparty-readerwriterqueue)

include(FetchContent)
FetchContent_Declare(
    readerwriterqueue
    GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue.git
    GIT_TAG        v1.0.7
)
FetchContent_MakeAvailable(readerwriterqueue)