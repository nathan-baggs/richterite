#pragma once

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>

#include <windows.h>

namespace richterite
{

inline auto get_temp(std::string_view filename) -> std::filesystem::path
{
    char tempPath[MAX_PATH]{};
    ::GetEnvironmentVariableA("TEMP", tempPath, MAX_PATH);
    return std::filesystem::path{tempPath} / filename;
}

template <class... T>
auto log(std::format_string<T...> fmt, T &&...args) -> void
{
    static auto mtx = std::mutex{};

    std::scoped_lock lock{mtx};

    const auto log_path = get_temp("log.txt");

    if (auto file = std::ofstream{log_path, std::ios::app}; file)
    {
        auto str = std::string{};
        std::format_to(std::back_inserter(str), fmt, std::forward<T>(args)...);
        file << str << std::endl;
    }
}

template <class... T>
auto die(std::format_string<T...> fmt, T &&...args) -> void
{
    log(fmt, std::forward<T>(args)...);
    std::cout << std::format(fmt, std::forward<T>(args)...) << std::endl;
    std::cout << std::flush;

    std::exit(1);
}

template <class... T>
auto ensure(bool cond, std::format_string<T...> fmt, T &&...args) -> void
{
    if (!cond)
    {
        die(fmt, std::forward<T>(args)...);
    }
}

}
