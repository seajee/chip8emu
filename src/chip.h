#ifndef _CHIP_H_
#define _CHIP_H_

#include <stdint.h>
#include <stdbool.h>

// Original CHIP-8 resolution
#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32

#define FOREGROUND_COLOR 0xFFFFFFFF
#define BACKGROUND_COLOR 0x000000FF

#define RAM_CAPACITY 4096
#define CHIP_ENTRY_POINT 0x200
#define MAX_ROM_SIZE RAM_CAPACITY - CHIP_ENTRY_POINT

#define CHIP_INST_PER_SECOND 500 // MHz (CHIP-8 "clock rate")

typedef struct Instruction
{
    uint16_t opcode;
    uint16_t NNN; // 12 bit address / constant
    uint8_t NN;   // 8 bit constant
    uint8_t N;    // 8 bit constant
    uint8_t X;    // 4 bit register identifier
    uint8_t Y;    // 4 bit register identifier
} Instruction;

typedef struct Chip8
{
    uint8_t ram[RAM_CAPACITY];
    bool display[WINDOW_WIDTH * WINDOW_HEIGHT];
    uint16_t stack[12];  // Subroutine stack
    uint16_t* stack_ptr;

    uint8_t V[16];       // Data registers V0-VF
    uint16_t I;          // Index register
    uint16_t PC;         // Program Counter

    uint8_t delay_timer; // Decrements at 60 Hz when above 0
    uint8_t sound_timer; // Decrements at 60 Hz and plays tone when above 0

    bool keypad[16];     // Hexadecimal keypad 0x0-0xF

    Instruction inst;    // Currently executing instruction
} Chip8;

bool chip8_init(Chip8* chip8, const char* rom_path);
void chip8_execute(Chip8* chip8);
void chip8_update_timers(Chip8* chip8);

#endif // _CHIP_H_

