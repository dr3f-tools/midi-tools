cmake_minimum_required(VERSION 3.27)
project(midi-tools)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(ALSA REQUIRED)

add_subdirectory(readerwriterqueue)
add_subdirectory(libremidi)

# set(RTMIDI_API_ALSA ON)
# set(RTMIDI_BUILD_TESTINGS OFF)
# add_subdirectory(rtmidi)

add_executable(midi-tools src/main.cpp)
target_link_libraries(midi-tools libremidi::libremidi)
# target_link_libraries(midi-tools rtmidi pthread)
# target_link_libraries(midi-tools asound)