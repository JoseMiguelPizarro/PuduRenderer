//
// Created by Administrator on 2/10/2025.
//

#pragma once

#include <string>
#include <string_view>
#include <format>


namespace StringUtils
{
    template<typename... Args>
    std::string Format(const std::format_string<Args...> fmt, Args&&... args)
    {
        return std::vformat(fmt.get(),  std::make_format_args(args...));
    }
}

