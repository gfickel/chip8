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

// Skip next instruction if Vx = kk.
TEST_CASE( "3xkk - SE Vx, byte" ) {
    unsigned short x  = 0x000F;
    unsigned short kk = 0x003E;

    // test Vx == kk
    prepare_test(0x3000 | (x << 8) | kk);
    unsigned short old_pc = chip8.pc;
    chip8.V[x] = (unsigned char)kk;

    chip8.runStep();

    REQUIRE( chip8.pc == old_pc+4 );


    // test Vx != kk
    prepare_test(0x3000 | (x << 8) | kk);
    old_pc = chip8.pc;
    chip8.V[x] = (unsigned char)kk-4;

    chip8.runStep();

    REQUIRE( chip8.pc == old_pc+2 );
}

// Skip next instruction if Vx != kk.
TEST_CASE( "4xkk - SNE Vx, byte" ) {
    unsigned short x  = 0x000F;
    unsigned short kk = 0x003E;

    // test Vx == kk
    prepare_test(0x4000 | (x << 8) | kk);
    unsigned short old_pc = chip8.pc;
    chip8.V[x] = (unsigned char)kk;

    chip8.runStep();

    REQUIRE( chip8.pc == old_pc+2 );

    // test Vx != kk
    prepare_test(0x4000 | (x << 8) | kk);
    old_pc = chip8.pc;
    chip8.V[x] = (unsigned char)kk-4;

    chip8.runStep();

    REQUIRE( chip8.pc == old_pc+4 );
}

// Set Vx = kk.
TEST_CASE( "6xkk - LD Vx, byte" ) {
    unsigned short x  = 0x000B;
    unsigned short kk = 0x003A;

    // test Vx == kk
    prepare_test(0x6000 | (x << 8) | kk);

    chip8.runStep();

    REQUIRE( (unsigned short)chip8.V[x] == kk );
}

// Set Vx = Vx + kk.
TEST_CASE( "7xkk - ADD Vx, byte" ) {
    unsigned short x  = 0x0008;
    unsigned short kk = 0x0005;
    unsigned char old_vx = 0x06;

    // test Vx += kk
    prepare_test(0x7000 | (x << 8) | kk);
    unsigned char old_vf = chip8.V[0xF];
    chip8.V[x] = old_vx; 

    chip8.runStep();

    // unsigned char + unsigned char == int (thanks, C++...)
    REQUIRE( chip8.V[x] == (unsigned char)(kk+old_vx) );
    REQUIRE( chip8.V[0xF] == old_vf );


    // test Vx += kk with overflow
    kk = 0x00FE;
    prepare_test(0x7000 | (x << 8) | kk);
    old_vf = chip8.V[0xF];
    chip8.V[x] = old_vx; 

    chip8.runStep();

    
    REQUIRE( chip8.V[x] == (unsigned char)(kk+old_vx) );
    REQUIRE( chip8.V[0xF] == old_vf );
}

// Set Vx = Vy.
TEST_CASE( "8xy0 - LD Vx, Vy" ) {
    unsigned short x  = 0x0003;
    unsigned short y  = 0x0005;
    unsigned short vy = 0x0043;

    prepare_test(0x8000 | (x << 8) | (y << 4));
    chip8.V[y] = vy;

    chip8.runStep();

    REQUIRE( chip8.V[x] == chip8.V[y] );
}


