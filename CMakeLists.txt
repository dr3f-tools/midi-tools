cmake_minimum_required(VERSION 3.27)
project(midi-tools)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(deps)

add_executable(midi-tools src/main.cpp)
target_link_libraries(midi-tools libremidi::libremidi)