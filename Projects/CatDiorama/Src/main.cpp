#define GLFW_INCLUDE_VULKAN

#include <iostream>
#include "CatDiorama.h"

int main() {
	CatDiorama app;

	try {
		app.Init();
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
