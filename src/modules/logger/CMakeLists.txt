project(logger LANGUAGES CXX)

add_subdirectory(tests)

add_library(logger STATIC
    logger.cpp
)
add_library(core::logger ALIAS logger)

target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
