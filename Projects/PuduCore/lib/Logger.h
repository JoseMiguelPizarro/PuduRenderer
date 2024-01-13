#pragma once
#include <iostream>

static void Print(const char* text, ...) {
	printf(text);
	printf("\n");
}


#define PUDU_ERROR(message)\
 throw std::runtime_error(std::format("Error: {}", message))

#define LOG(message)\
Print(message)

//#define LOG(message)\
//Print(message.c_str())