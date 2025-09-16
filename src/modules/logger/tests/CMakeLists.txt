project(logger_tests LANGUAGES CXX)

add_executable(logger_tests
    logger.tests.cpp
    main.cpp
)
target_link_libraries(logger_tests PRIVATE
    logger
    doctest::doctest
)
add_test(NAME logger_tests COMMAND logger_tests)