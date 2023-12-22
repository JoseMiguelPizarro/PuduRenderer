#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "hlsl++.h"
#include <string>
#include <PuduApp.h>
#include <iostream>

int main() {

	PuduApp app;

	try {
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
