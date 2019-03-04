#include "chip8.h"
#include "catch2/catch.hpp"


Chip8 chip8;

void prepare_test(unsigned short op) {
    chip8.ram[512] = (op & 0xFF00) >> 8;
    chip8.ram[513] = op & 0x00FF;
    chip8.pc = 512;
    chip8.game_max_address = 1024;
    chip8.stack_pointer = 0;
}

// Clear screen
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

// Return from a subroutine.
TEST_CASE( "00EE - RET" ) {
    prepare_test(0x00EE);
    chip8.stack_pointer = 1;
    chip8.stack[0] = 600;

    chip8.runStep();

    REQUIRE( chip8.pc == 602 );
}


// Jump to location nnn
TEST_CASE( "1nnn - JP addr" ) {
    unsigned short address = 0x012F;
    prepare_test(0x1000 | address);
    
    chip8.runStep();

    REQUIRE( chip8.pc == address );
}

// Call subroutine at nnn.
TEST_CASE( "2nnn - CALL addr" ) {
    unsigned short address = 0x022F;
    prepare_test(0x2000 | address);
    
    unsigned short old_pc = chip8.pc;
    chip8.runStep();

    REQUIRE( chip8.stack[0] == old_pc );
    REQUIRE( chip8.pc == address );

}
