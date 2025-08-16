#include <SFML/Graphics.hpp>
#include "../include/GameEngine.h"


int main() {
    GameEngine g("assets.txt");
    g.run();

    return 0;
}
