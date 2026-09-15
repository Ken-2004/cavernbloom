#include "Game.hpp"
#include "RuntimePaths.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
    try {
        cavernbloom::Game game(cavernbloom::executableDirectory());
        game.run();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "CavernBloom: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
