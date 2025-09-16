#pragma once

#include <format>
#include <source_location>
#include <string>

namespace logger
{

void log(std::string const& msg, std::source_location const& loc = std::source_location::current());

template <typename... Args>
void log(std::format_string<Args...> auto const& fmt, Args&&... args) {
    log(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}

}