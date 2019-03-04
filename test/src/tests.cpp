#define CATCH_CONFIG_MAIN

#include "chip8.h"
#include "catch2/catch.hpp"


Chip8 chip8;

void prepare_test(unsigned short op) {
    chip8.ram[512] = (op & 0xFF00) >> 8;
    chip8.ram[513] = op & 0x00FF;
    chip8.pc = 512;
    chip8.game_max_address = 1024;
}

TEST_CASE( "00E0 - CLS" ) {
    prepare_test(0x00E0);
    for (int i=0; i<32; i++) {
        for (int j=0; j<64; j++) {
            chip8.display[i][j] = 27; 
        }
    }

    chip8.runStep();
    
    bool all_blank = true;
    for (int i=0; i<32; i++) {
        for (int j=0; j<64; j++) {
            if (chip8.display[i][j] > 0) {
                all_blank = false;
            }
        }
    }

    REQUIRE( all_blank == true );
}

