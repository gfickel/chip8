#include <stdio.h>
#include "chip8.h"

int main(int argc, char* argv[]) {
    Chip8 chip8;

    while (true) {
        chip8.runStep();
    }

    return 0;
}
