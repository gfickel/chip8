#include "chip8.h"


Chip8::Chip8() {
    clock  = 500; 
    opcode = 0;
    pc     = 0x200;
    I      = 0;
    stack_pointer   = 0;
    display_updated = true;

    sound_timer = 0;
    delay_timer = 0;
    game_max_address = 81;

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

    for (int i=0; i<4096; i++) {
        ram[i] = 0;
    }

    for (int i=0; i<80; i++) {
        ram[i] = font_aux[i];
    }

    for (int i=0; i<16; i++) {
        V[i]    = 0;
        keys[i] = 0;
    }

    for (int i=0; i<32; i++) {
        for (int j=0; j<64; j++) {
            display[i][j] = 0;
        }
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
    game_max_address = 512+length;

    fin.read((char*)&ram[512], length);

    return true;
}


void Chip8::runStep() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> ellapsed_fetch = now-last_fetch;
    std::chrono::duration<double, std::micro> ellapsed_timer = now-last_timer;
    last_timer = now;
    last_fetch = now;

    // update timers
    if ((int)ellapsed_timer.count() >= 1000.0/60.0*1000000) {
        sound_timer++;
        delay_timer++;
        if (sound_timer >= 60) {
            sound_timer = 0;
            delay_timer = 0;
        }
    }

    // sleep according to the clock
    float time_to_sleep = (1000.0/clock)*1000 - ellapsed_fetch.count()*1000000;
    if (time_to_sleep > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(int(time_to_sleep)));
    }

    // Fetch + run instruction
    opcode = ram[pc] << 8 | ram[pc + 1];
    // printf("============================================\n");
    // printf("PC: %d\tgame_max_address: %d\n", pc, game_max_address);
    if (pc >= game_max_address) {
        exit(1);
    }
    // printf("OPCODE: %#06x\n", opcode);

    switch(opcode & 0xF000) {
        case 0x0000: 
            {
                switch (opcode & 0x000F) {
                    case 0x0000: // CLS
                        for (int i=0; i<32; i++) {
                            for (int j=0; j<64; j++) {
                                display[i][j] = 0;
                            }
                        }
                        display_updated = true;
                        pc += 2;
                        break;
                    case 0x000E: // RET
                        pc = stack[--stack_pointer];
                        break;
                    default:
                        printf("Undefined OP Code");
                        exit(1);
                }
            }
            break;
        
        case 0x1000: // 1nnn - JP addr
            pc = opcode & 0x0FFF;
            break;
        
        case 0x2000: // 2nnn - CALL addr
            stack[stack_pointer] = pc;
            ++stack_pointer;
            pc = opcode & 0x0FFF;
            break;
        
        case 0x6000: // 6xkk - LD Vx, byte 
            {
                unsigned char register_id = (opcode & 0x0F00) >> 8;
                V[register_id] = opcode & 0x00FF;
                pc += 2;
            }
            break;
        
        case 0x7000: // 7xkk - ADD Vx, byte 
            {
                unsigned char x = (opcode & 0x0F00) >> 8;
                V[x] += (opcode & 0x00FF);
                pc += 2;
            }
            break;

        case 0x9000: // 9xy0 - SNE Vx, Vy
            if (V[(opcode&0x0F00) >> 8] != V[(opcode & 0x00F0)>>4]) {
               pc += 2; 
            }
            pc += 2;
            break;

        case 0xA000: // Annn - LD I, addr
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        
        case 0xD000: // Dxyn - DRW Vx, Vy, nibble
        {
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;
            
            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = ram[I + yline];
                for(int xline = 0; xline < 8; xline++) {
                    if((pixel & (0x80 >> xline)) != 0) {
                        if(display[y + yline][x + xline] == 1)
                            V[0xF] = 1;                                 
                        display[y + yline][x + xline] ^= 1;
                    }
                }
            }
            display_updated = true;

            // for (int i=0; i<32; i++) {
            //     for (int j=0; j<64; j++) {
            //         if (display[i][j] > 0) {
            //             printf("X");
            //         } else {
            //             printf(" ");
            //         }
            //     }
            //     printf("\n");
            // }
            // printf("\n\n\n\n");
            
            pc += 2;
            
        }
        break;
        
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: // Fx07 - LD Vx, DT
                    V[(opcode & 0x0F00)>>8] = delay_timer;
                    pc += 2;
                    break;
                
                case 0x0029: // Fx29 - LD F, Vx
                {
                    unsigned char x = V[(opcode & 0x0F00) >> 8];
                    I = x*4;
                    pc += 2;
                }
                    break;
                
                case 0x0033: // Fx33 - LD B, Vx
                {
                    unsigned char x = V[(opcode & 0x0F00) >> 8];
                    ram[I+0] = int(x)/100;
                    ram[I+1] = (int(x)-int(ram[I+0]))/10;
                    ram[I+2] = int(x) % 10;
                    pc += 2;
                }
                    break;
                
                case 0x0065: // Fx65 - LD Vx, [I]
                {
                    unsigned short x = (opcode & 0x0F00) >> 8;
                    for (int i=0; i<=x; i++) {
                        V[i] = ram[I+i];
                    }
                    pc += 2;
                }
                    break;
                
                default:
                    printf("Invalid OP code\n");
                    exit(1);
            }
        
            break;

        default: 
            printf("Bad instruction: %#06x\n", opcode & 0xF000);
            exit(1);
    }

    last_fetch = std::chrono::high_resolution_clock::now();
}
