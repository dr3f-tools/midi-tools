#pragma once

#include <format>
#include <source_location>
#include <string>

namespace logger
{

void log(std::string const& msg, std::source_location const& loc = std::source_location::current());

template <typename... Args>
void log(std::format_string<Args...> fmt, Args&&... args) {
    log(std::format(fmt, std::forward<Args>(args)...));
}

// template <typename... Args>
// constexpr void log(char const* fmt, Args&&... args) {
//     log(std::vformat(fmt, std::forward<Args>(args)...));
// }

}