//
// Created by Administrator on 4/12/2025.
//
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>

#include "Test_PBR.h"
#include "Logger.h"
int main() {

    Test_PBR app;

    try {
        app.Init();
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    return EXIT_SUCCESS;
}
