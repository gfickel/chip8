#include <stdio.h>
#include "chip8.h"

int main(int argc, char* argv[]) {
    Chip8 chip8;

    if (argc != 2) {
        printf("Usage: ./chip8 path/to/game/awesomegame\n");
        return 0;
    }

    if (chip8.loadGame(argv[1]) == false) {
        printf("Problem loading the provided game: %s\n", argv[1]);
        return 1;
    }

    while (true) {
        chip8.runStep();
    }

    return 0;
}
