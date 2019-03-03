#include <chrono>
#include <thread>


typedef struct Chip8 {
    unsigned short opcode;
    unsigned char ram[4096];
    unsigned char v[16]; // CPU registers, from V0 to VE, with VF being for special cases
    unsigned short pc;
    unsigned short I; // Memory address register
    unsigned int clock; // Hz

    unsigned short stack[24];
    unsigned short stack_pointer;

    unsigned char sound_timer; // Both timers operate at 60Hz, and at 60 they return to 0
    unsigned char delay_timer;

    unsigned char keys[16];
    unsigned char display[32][64];
    unsigned char font[80];

    std::chrono::time_point<std::chrono::high_resolution_clock> last_fetch; 
    std::chrono::time_point<std::chrono::high_resolution_clock> last_timer; 
  

    Chip8();

    void runStep();

} Chip8;

