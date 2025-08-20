#include <SFML/Graphics.hpp>
#include "../include/GameEngine.h"


int main() {
    GameEngine g("../bin/assets.txt");
    g.run();

    return 0;
}
