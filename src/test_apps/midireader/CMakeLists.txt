cmake_minimum_required(VERSION 3.27)
project(midireader)

add_executable(midireader midireader.cpp)
target_link_libraries(midireader PRIVATE
    libremidi::libremidi
)