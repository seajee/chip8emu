#include "chip.h"

#include <stdio.h>
#include <string.h>

#include "font.h"

bool chip8_init(Chip8* chip8, const char* rom_path)
{
    // Initialize entire CHIP-8 machine
    memset(chip8, 0, sizeof(Chip8));

    // Set CHIP-8 machine defaults
    chip8->PC = CHIP_ENTRY_POINT;
    chip8->stack_ptr = &chip8->stack[0];

    // Load font
    memcpy(&chip8->ram[0], g_font, sizeof(g_font));

    // Load ROM
    FILE* rom_file = fopen(rom_path, "rb");

    if (!rom_file) {
        fprintf(stderr, "ERROR: Could not open ROM file \"%s\"\n", rom_path);
        return false;
    }

    fseek(rom_file, 0, SEEK_END);
    const long rom_size = ftell(rom_file);
    rewind(rom_file);

    if (rom_size > MAX_ROM_SIZE) {
        fprintf(stderr, "ERROR: ROM size too big\n");
        return false;
    }

    if (fread(&chip8->ram[CHIP_ENTRY_POINT], rom_size, 1, rom_file) != 1) {
        fprintf(stderr, "ERROR: Could not load ROM file into memory\n");
        return false;
    }

    fclose(rom_file);

    return true;
}

#ifdef DEBUG
void print_debug_info(Chip8* chip8)
{
    printf("Address: 0x%04X, Opcode: 0x%04X Desc: ", chip8->PC - 2, chip8->inst.opcode);

    // Emulate opcode
    switch (chip8->inst.opcode >> 12) {
    case 0x00:
        switch (chip8->inst.NN) {
        case 0xE0:
            // 0x00E0: Clear the screen
            printf("Clear the screen\n");
            break;

        case 0xEE:
            // 0x00EE: Return from a subroutine
            // Pop last addres from the stack
            //  and set PC to it
            printf("Return from subroutine\n");
            break;
        
        default:
            printf("Not implemented\n");
            break;
        }
        break;
    
    case 0x01:
        // 0x1NNN: Jump to address NNN
        printf("Jump to address NNN (0x%04X)\n", chip8->inst.NNN);
        break;
    
    case 0x02:
        // 0x2NNN: Call subroutine at NNN
        // Push PC in the stack and set PC
        //   to the jump address
        printf("Call subroutine\n");
        break;

    case 0x06:
        // 0x6XNN: Set VX to NN
        printf("Set V%X to NN (0x%02X)\n", chip8->inst.X, chip8->inst.NN);
        break;

    case 0x07:
        // 0x7XNN: Adds NN to VX (carry flag is not changed)
        printf("Adds NN (0x%02X) to V%X (0x%02X)\n", chip8->inst.NN, chip8->inst.X, chip8->V[chip8->inst.X]);
        break;

    case 0x0A:
        // 0xANNN: Set I to the address NNN
        printf("Sets I to NNN (0x%04X)\n", chip8->inst.NNN);
        break;

    case 0x0D:
        // 0xDXYN: Draw a sprite at coordinate (VX, VY)
        //  Read from memory location I.
        //  VF (Carry flag) is set if any screen pixels are set off
        printf("Draw N (%u) height sprite at coords "
            "V%X (0x%02X), V%X (0x%02X) from memory location I (0x%04X)\n",
            chip8->inst.N, chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.Y, chip8->V[chip8->inst.Y], chip8->I);
        break;

    default:
        printf("Not implemented\n");
        break; // Not implemented or invalid opcode
    }
}
#endif

void chip8_execute(Chip8* chip8)
{
    // Get next opcode from RAM
    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | (chip8->ram[chip8->PC + 1]);

    // Increment Program Counter for next opcode
    chip8->PC += 2;

    // Fill out current instruction format
    chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
    chip8->inst.NN = chip8->inst.opcode & 0x0FF;
    chip8->inst.N = chip8->inst.opcode & 0x0F;
    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

#ifdef DEBUG
    print_debug_info(chip8);
#endif

    // Emulate opcode
    switch (chip8->inst.opcode >> 12) {
    case 0x00:
        switch (chip8->inst.NN) {
        case 0xE0:
            // 0x00E0: Clear the screen
            memset(&chip8->display[0], false, sizeof(chip8->display));
            break;

        case 0xEE:
            // 0x00EE: Return from a subroutine
            // Pop last addres from the stack
            //  and set PC to it
            chip8->PC = *--chip8->stack_ptr;
            break;
        }
        break;

    case 0x01:
        // 0x1NNN: Jump to address NNN
        chip8->PC = chip8->inst.NNN;
        break;
    
    case 0x02:
        // 0x2NNN: Call subroutine at NNN
        // Push PC in the stack and set PC
        //   to the jump address
        *chip8->stack_ptr++ = chip8->PC;
        chip8->PC = chip8->inst.NNN;
        break;
    
    case 0x06:
        // 0x6XNN: Set VX to NN
        chip8->V[chip8->inst.X] = chip8->inst.NN;
        break;

    case 0x07:
        // 0x7XNN: Adds NN to VX (carry flag is not changed)
        chip8->V[chip8->inst.X] += chip8->inst.NN;
        break;
    
    case 0x0A:
        // 0xANNN: Set I to the address NNN
        chip8->I = chip8->inst.NNN;
        break;
    
    case 0x0D:
        // 0xDXYN: Draw a sprite at coordinate (VX, VY)
        //  Read from memory location I.
        //  VF (Carry flag) is set if any screen pixels are set off
        uint8_t x_coord = chip8->V[chip8->inst.X] % WINDOW_WIDTH;
        uint8_t y_coord = chip8->V[chip8->inst.Y] % WINDOW_HEIGHT;
        const uint8_t orig_x = x_coord; // Original X value

        chip8->V[0xF] = 0; // Initialize Carry flag

        for (uint8_t i = 0; i < chip8->inst.N; ++i) {
            // Get next byte / row of sprite data
            const uint8_t sprite_data = chip8->ram[chip8->I + i];
            x_coord = orig_x; // Reset X for next row to draw

            for (int8_t j = 7; j >= 0; j--) {
                bool* pixel = &chip8->display[y_coord * WINDOW_WIDTH + x_coord];
                const bool sprite_bit = sprite_data & (1 << j);

                // If sprite pixel / bit is on and display pixel is on, set carry flag
                if (sprite_bit && *pixel) {
                    chip8->V[0xF] = 1;
                }

                // XOR display pixel with sprite pixel / bit
                *pixel ^= sprite_bit;

                // Stop drawing if hit right edge of the screen
                if (++x_coord >= WINDOW_WIDTH) {
                    break;
                }
            }

            // Stop drawing sprite if hit bottom edge of screen
            if (++y_coord >= WINDOW_HEIGHT) {
                break;
            }
        }

        break;

    default:
        break; // Not implemented or invalid opcode
    }
}

