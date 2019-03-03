#include "chip8.h"


Chip8::Chip8() {
    clock  = 500; 
    opcode = 0;
    pc     = 0x200;
    I      = 0;
    stack_pointer = 0;

    sound_timer = 0;
    delay_timer = 0;

    unsigned char font_aux[80] = { 
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i=0; i<80; i++) {
        ram[i] = font_aux[i];
    }

    for (int i=0; i<4096; i++) {
        ram[i] = 0;
    }

    for (int i=0; i<16; i++) {
        V[i] = 0;
    }

    last_fetch = std::chrono::high_resolution_clock::now();
    last_timer = std::chrono::high_resolution_clock::now();
};


bool Chip8::loadGame(const char* fileName) {
    printf("Loading game %s\n", fileName);

    std::ifstream fin(fileName, std::ios::binary|std::ios::ate);
    std::ifstream::pos_type pos = fin.tellg();
    int length = pos;
    fin.seekg(0, std::ios::beg);
    if (length > 4096-512) {
        return false;
    }

    fin.read((char*)&ram[512], length);

    return true;
}


void Chip8::runStep() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> ellapsed_fetch = now-last_fetch;
    std::chrono::duration<double, std::micro> ellapsed_timer = now-last_timer;

    // update timers
    if ((int)ellapsed_timer.count() >= 1000.0/60.0*1000000) {
        sound_timer++;
        delay_timer++;
        if (sound_timer >= 60) {
            sound_timer = 0;
            delay_timer = 0;
        }
        last_timer = now;
    }

    // sleep according to the clock
    float time_to_sleep = (1000.0/clock)*1000 - (int)ellapsed_fetch.count()*1000000;
    if (time_to_sleep > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(int(time_to_sleep/1000000.0)));
    }

    // Fetch + run instruction

    last_fetch = std::chrono::high_resolution_clock::now();
}
