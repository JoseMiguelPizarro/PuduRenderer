#pragma once
#include <fmt/core.h>


static void Print(const char* text, ...) {
	printf(text);
	printf("\n");
}

#define PUDU_ERROR(message,...)\
 throw std::runtime_error(fmt::format("Error: {}",fmt::format( message,__VA_ARGS__)))\

#define LOG(message,...)\
fmt::print(message,__VA_ARGS__);\
fmt::print("\n")\

//#define LOG(message)\
//Print(message.c_str())