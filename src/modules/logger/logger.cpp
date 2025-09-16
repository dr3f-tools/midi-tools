#include "logger/logger.h"

#include <iostream>

namespace
{

/// @brief Extract filename from source location, removing all directories up to the specified level.
static constexpr std::string_view filename(std::source_location const& loc, int level = 0) {
    std::string_view file = loc.file_name();
    size_t pos = file.size();
    for (int i = 0; i <= level; ++i) {
        auto substr = file.substr(0, pos);
        auto pos2 = substr.rfind('/');
        if (pos2 == std::string_view::npos) {
            break;
        }
        pos = pos2;
    }
    if (pos == file.size()) {
        return file;
    }
    return file.substr(pos + 1, file.size() - pos - 1);
}

/// @brief Extract function name from source location
static constexpr std::string_view funcname(std::source_location const& loc) {
    std::string_view func = loc.function_name();
    auto pos = func.find('(');
    if (pos != std::string_view::npos) {
        func = func.substr(0, pos);
    }
    // pos = func.rfind("::");
    // if (pos != std::string_view::npos) {
    //     func = func.substr(pos + 2);
    // }
    pos = func.rfind(" ");
    if (pos != std::string_view::npos) {
        func = func.substr(pos + 1);
    }
    return func;
}

}  // namespace

namespace logger
{

void log(std::string const& msg, std::source_location const& loc) {
    std::cout << filename(loc, 1) << "(" << loc.line() << ")" << "::" << funcname(loc);
    if (!msg.empty()) {
        std::cout << ": " << msg;
    }
    std::cout << std::endl;
}

}  // namespace logger