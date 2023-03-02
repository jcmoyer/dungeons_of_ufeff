#include <SDL.h>

#include "game.hpp"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    game g;
    g.run();

    return 0;
}