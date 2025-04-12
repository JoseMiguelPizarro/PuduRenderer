#pragma once
#include <format>
#include "PuduCore.h"

namespace Pudu
{
    inline const char* k_red = "\033[31m";
    inline const char* k_yellow = "\033[33m";
    inline const char* k_green = "\033[32m";
    inline const char* k_reset = "\033[0m";


    static void Print(const char* text, ...)
    {
        printf(text);
        printf("\n");
    }

    template <typename... Args>
    void LOG(const std::format_string<Args...> fmt, Args&&... args)
    {
        auto formattedMessage = std::vformat(fmt.get(), std::make_format_args(args...));

        std::printf(formattedMessage.c_str());
        std::printf("\n");
    }

    template <typename... Args>
    void LOG_ERROR(const std::format_string<Args...> fmt, Args&&... args)
    {
        auto formattedMessage = std::vformat(fmt.get(), std::make_format_args(args...));

        std::printf(std::format("{}🐞Error 👉👈: {} {}", k_red, formattedMessage, k_reset).c_str());
        std::printf("\n");
    }

    template <typename... Args>
    void LOG_WARNING(const std::format_string<Args...> fmt, Args&&... args)
    {
        auto formattedMessage = std::vformat(fmt.get(), std::make_format_args(args...));

        std::printf(std::format("{}⚠️: {}{}",k_yellow,formattedMessage,k_reset).c_str());
        std::printf("\n");
    }

    // Assert function to validate expressions
    template <typename... Args>
    void ASSERT(bool condition, const std::format_string<Args...> fmt, Args&&... args)
    {
        if (!condition)
        {
            auto formattedMessage = std::vformat(fmt.get(), std::make_format_args(args...));
            throw std::runtime_error(std::vformat("Assertion failed 🫤: {0}", std::make_format_args(formattedMessage)));
        }
    }

    template <typename... Args>
    void LOG_I(Pudu::u32 indentation, const std::format_string<Args...> fmt, Args&&... args)
    {
        std::string indentationSpaces(indentation, '\t'); // Create spaces for indentation (1 tab per level)
        auto formattedMessage = std::vformat(fmt.get(), std::make_format_args(args...)); // Format the message
        std::printf("%s%s\n", indentationSpaces.c_str(), formattedMessage.c_str()); // Print the indented message
    }
}
