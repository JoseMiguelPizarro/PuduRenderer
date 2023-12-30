#pragma once
#include <iostream>

using namespace std;

static void Print(const char* text, ...) {
	printf(text);
	printf("\n");
}


#define PUDU_ERROR(message)\
 throw std::runtime_error(std::format("Error: {}", message))
