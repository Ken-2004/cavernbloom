#include "Game.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
    try {
        cavernbloom::Game game;
        game.run();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "CavernBloom: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
