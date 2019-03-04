#include "chip8.h"
#include "catch2/catch.hpp"


Chip8 chip8;

void prepare_test(unsigned short op) {
    chip8.ram[512] = (op & 0xFF00) >> 8;
    chip8.ram[513] = op & 0x00FF;
    chip8.pc = 512;
    chip8.game_max_address = 1024;
    chip8.stack_pointer = 0;
    chip8.I = 0x0000;
    for (int i=0; i<16; i++) chip8.keys[i] = 0;
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

// Set Vx = Vx AND Vy.
TEST_CASE( "8xy2 - AND Vx, Vy" ) {
    unsigned short x  = 0x000A;
    unsigned short y  = 0x0005;
    unsigned short vx = 0x0003;
    unsigned short vy = 0x0043;

    prepare_test(0x8002 | (x << 8) | (y << 4));
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();

    REQUIRE( chip8.V[x] == (vx & vy) );
}

// Set Vx = Vx + Vy, set VF = carry.
TEST_CASE( "8xy4 - ADD Vx, Vy" ) {
    unsigned short x  = 0x000A;
    unsigned short y  = 0x0005;
    unsigned short vx = 0x0003;
    unsigned short vy = 0x0043;

    // Addition without carry
    prepare_test(0x8004 | (x << 8) | (y << 4));
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();

    REQUIRE( chip8.V[x] == (unsigned char)(vx + vy) );
    REQUIRE( chip8.V[0xF] == 0 );
    
    // Addition with carry
    vx = 0x00AA;
    vy = 0x00BB;

    prepare_test(0x8004 | (x << 8) | (y << 4));
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();

    REQUIRE( chip8.V[x] == (unsigned char)(vx + vy) );
    REQUIRE( chip8.V[0xF] == 1 );
}

// Set Vx = Vx - Vy, set VF = NOT borrow.
TEST_CASE( "8xy5 - SUB Vx, Vy" ) {
    unsigned short x  = 0x000C;
    unsigned short y  = 0x0002;
    unsigned short vx = 0x0050;
    unsigned short vy = 0x0004;

    // Subtraction without carry
    prepare_test(0x8005 | (x << 8) | (y << 4));
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();

    REQUIRE( chip8.V[x] == (unsigned char)(vx - vy) );
    REQUIRE( chip8.V[0xF] == 1 );
    
    // Subtraction with carry
    vx = 0x00AA;
    vy = 0x00BB;

    prepare_test(0x8005 | (x << 8) | (y << 4));
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();

    REQUIRE( chip8.V[x] == (unsigned char)(vx - vy) );
    REQUIRE( chip8.V[0xF] == 0 );
}

// Set Vx = Vy - Vx, set VF = NOT borrow.
TEST_CASE( "8xy7 - SUBN Vx, Vy" ) {
    unsigned short x  = 0x000C;
    unsigned short y  = 0x0002;
    unsigned short vx = 0x0004;
    unsigned short vy = 0x0064;

    // Subtraction without carry
    prepare_test(0x8007 | (x << 8) | (y << 4));
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();

    REQUIRE( chip8.V[x] == (unsigned char)(vy - vx) );
    REQUIRE( chip8.V[0xF] == 1 );
    
    // Subtraction with carry
    vx = 0x00CC;
    vy = 0x0003;

    prepare_test(0x8007 | (x << 8) | (y << 4));
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();

    REQUIRE( chip8.V[x] == (unsigned char)(vy - vx) );
    REQUIRE( chip8.V[0xF] == 0 );
}

// Skip next instruction if Vx != Vy.
TEST_CASE( "9xy0 - SNE Vx, Vy" ) {
    unsigned short x  = 0x0003;
    unsigned short y  = 0x0005;
    unsigned short vx = 0x00CE;
    unsigned short vy = 0x00CE;

    prepare_test(0x9000 | (x << 8) | (y << 4));
    unsigned short old_pc = chip8.pc;
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();
    REQUIRE( chip8.pc == old_pc+2 );

    vx = 0x00AA;
    prepare_test(0x9000 | (x << 8) | (y << 4));
    old_pc = chip8.pc;
    chip8.V[x] = vx;
    chip8.V[y] = vy;

    chip8.runStep();
    REQUIRE( chip8.pc == old_pc+4 );
}

// Set I = nnn.
TEST_CASE( "Annn - LD I, addr" ) {
    unsigned short nnn = 0x0ABC;

    prepare_test(0xA000 | nnn);

    chip8.runStep();

    REQUIRE( chip8.I == nnn );
}

// Set Vx = random byte AND kk.
TEST_CASE( "Cxkk - RND Vx, byte" ) {
    unsigned short x  = 0x000C;
    unsigned short vx = 0x000F;
    unsigned short kk = 0x00BE;

    prepare_test(0xC000 | (x << 8) | kk);
    chip8.V[x] = vx;
    srand(42);
    chip8.runStep();
    REQUIRE( chip8.V[x] == 0x0006 );
}

// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
// Dxyn - DRW Vx, Vy, nibble


// Skip next instruction if key with the value of Vx is pressed.
TEST_CASE( "Ex9E - SKP Vx" ) {
    unsigned short x = 0x000A;

    prepare_test(0xE09E | (x << 8));
    unsigned short old_pc = chip8.pc;
    chip8.V[x] = x;
    chip8.runStep();
    REQUIRE( chip8.pc == old_pc+2 );
    
    prepare_test(0xE09E | (x << 8));
    chip8.V[x] = x;
    chip8.keys[x] = 1;
    old_pc = chip8.pc;
    chip8.runStep();
    REQUIRE( chip8.pc == old_pc+4 );
}

// Skip next instruction if key with the value of Vx is not pressed.
TEST_CASE( "ExA1 - SKNP Vx" ) {
    unsigned short x = 0x000A;

    prepare_test(0xE0A1 | (x << 8));
    unsigned short old_pc = chip8.pc;
    chip8.V[x] = x;
    chip8.runStep();
    REQUIRE( chip8.pc == old_pc+4 );
    
    prepare_test(0xE0A1 | (x << 8));
    chip8.V[x] = x;
    chip8.keys[x] = 1;
    old_pc = chip8.pc;
    chip8.runStep();
    REQUIRE( chip8.pc == old_pc+2 );
}
