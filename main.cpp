#include <iostream>

#include "app.h"

int main() {
    std::cout << "Hello, World! Let us play a game" << std::endl;
    try {
        App game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr<<"The game shut down after catching exception: "<<e.what()<<std::endl;
    }
    std::cout<<"Farewell, world"<<std::endl;
    return 0;
}
