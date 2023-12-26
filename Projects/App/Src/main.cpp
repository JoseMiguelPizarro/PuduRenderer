#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "hlsl++.h"
#include <string>
#include <TriangleApp.h>
#include <iostream>

int main() {

	TriangleApp app;

	try {
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
