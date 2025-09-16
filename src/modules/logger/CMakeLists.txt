project(logger LANGUAGES CXX)

add_library(logger STATIC
    logger.cpp
)
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(add_orphan_headers)
add_orphan_headers(logger
    include/logger/logger.h
)