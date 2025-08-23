//
// Created by Administrator on 8/23/2025.
//

//
// Created by Hojaverde on 4/12/2025.
//
#include <iostream>

#include "NewPuduAppTemplate.h"
int main() {

    NewPuduAppTemplate app;

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
