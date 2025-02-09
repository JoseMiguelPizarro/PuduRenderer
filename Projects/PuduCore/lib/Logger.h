#pragma once
#include <format>


static void Print(const char* text, ...) {
	printf(text);
	printf("\n");
}


template<typename... Args>
void LOG_ERROR(const std::format_string<Args...> fmt, Args&&... args) {

	auto formattedMessage = std::vformat(fmt.get(),  std::make_format_args(args...));

	throw std::runtime_error(std::vformat("Error 👉👈: {0} \n", std::make_format_args(formattedMessage)));
	std::abort();
}

template<typename... Args>
void LOG(const std::format_string<Args...> fmt, Args&&... args) {

	auto formattedMessage = std::vformat(fmt.get(),  std::make_format_args(args...));

	std::printf(formattedMessage.c_str());
	std::printf("\n");
}

//#define LOG(message)\
//Print(message.c_str())