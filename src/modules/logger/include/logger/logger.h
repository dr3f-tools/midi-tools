#pragma once

#include <format>
#include <source_location>
#include <string>

namespace logger
{
namespace detail
{
void log(std::string const& msg, std::source_location const& loc = std::source_location::current());
}

struct FormatWithLocation
{
    std::string_view value;
    std::source_location loc;

    constexpr FormatWithLocation(
        const char* s,
        const std::source_location& l = std::source_location::current()
    )
    : value(s)
    , loc(l) {}
};

template <typename... Args>
void log(FormatWithLocation fmt, Args&&... args) {
    auto formatted = std::vformat(fmt.value, std::make_format_args(args...));
    detail::log(formatted, fmt.loc);
}

}  // namespace logger