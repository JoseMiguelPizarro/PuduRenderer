//
// Created by Administrator on 8/23/2025.
//

//
// Created by Hojaverde on 4/12/2025.
//
#include <iostream>

#include "Blender_HotReload.h"
int main() {

    Blender_HotReload app;

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
