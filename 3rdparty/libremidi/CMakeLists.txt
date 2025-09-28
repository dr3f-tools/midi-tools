project(3rdparty-libremidi)

find_package(ALSA REQUIRED)
find_package(Boost REQUIRED)

add_compile_options(
    -Wno-maybe-uninitialized # silence a Boost warning from libremidi
)

set(LIBREMIDI_EXAMPLES ON)
if (LIBREMIDI_EXAMPLES)
    # Errors in examples
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-catch-value>
        -Wno-missing-field-initializers
        -Wno-unused-label
    )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/libremidi)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/libremidi)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib_static/libremidi)

include(FetchContent)
FetchContent_Declare(
    libremidi
    GIT_REPOSITORY  https://github.com/celtera/libremidi
    GIT_TAG         v5.3.1
)
FetchContent_MakeAvailable(libremidi)
