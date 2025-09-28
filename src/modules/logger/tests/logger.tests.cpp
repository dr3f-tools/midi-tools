#include "logger/logger.h"

#include <doctest/doctest.h>

TEST_CASE("logger basic") {
    logger::log("Hello, World!");
    logger::log("Formatted number: {}", int(42));
    logger::log("Multiple values: {}, {}, {}", 1, 2.5, "test");
}

TEST_CASE("logger source location") {
    logger::log("This log should show the correct file and line number.");
    auto lambda = []() { logger::log("Log from inside a lambda function."); };
    lambda();
}