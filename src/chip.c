#include "chip.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "font.h"

bool chip8_init(Chip8* chip8, const char* rom_path)
{
    // Initialize entire CHIP-8 machine
    memset(chip8, 0, sizeof(Chip8));

    // Set the seed for RNG
    srand(time(NULL));

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
            // Pop last address from the stack
            //  and set PC to it
            printf("Return from subroutine\n");
            break;

        default:
            // Not implemented
            //   or 0x0NNN: Calls machine code routine at address NNN
            break;
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
        printf("Call subroutine at NNN (0x%04X)\n", chip8->inst.NNN);
        break;

    case 0x03:
        // 0x3XNN: Skip the next instruction if VX equals NN
        printf("Skip the next instruction if V%X (0x%02X) equals NN (0x%02X)\n",
            chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.NN);
        break;

    case 0x04:
        // 0x4XNN: Skip the next instruction if VX does not equal NN
        printf("Skip the next instruction if V%X (0x%02X) does not equal NN (0x%02X)\n",
            chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.NN);
        break;

    case 0x05:
        // 0x5XY0: Skip the next instruction if VX equals VY
        printf("Skip the next instruction if V%X (0x%02X) equals V%X (0x%02X)\n",
            chip8->inst.X, chip8->V[chip8->inst.X],
            chip8->inst.Y, chip8->V[chip8->inst.Y]);
        break;

    case 0x06:
        // 0x6XNN: Set VX to NN
        printf("Set V%X to NN (0x%02X)\n", chip8->inst.X, chip8->inst.NN);
        break;

    case 0x07:
        // 0x7XNN: Adds NN to VX (carry flag is not changed)
        printf("Adds NN (0x%02X) to V%X (0x%02X)\n",
            chip8->inst.NN, chip8->inst.X, chip8->V[chip8->inst.X]);
        break;

    case 0x08:
        switch (chip8->inst.N) {
        case 0x0:
            // 0x8XY0: Set VX to the value of VY
            chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y];
            printf("Set V%X (0x%02X) to the value of V%X (0x%02X)\n",
                chip8->inst.X, chip8->V[chip8->inst.X],
                chip8->inst.Y, chip8->V[chip8->inst.Y]);
            break;

        case 0x1:
            // 0x8XY1: Set VX to VX or VY (bitwise)
            printf("Set V%X (0x%02X) to V%X or V%X (0x%02X) (bitwise)\n",
                chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.X,
                chip8->inst.Y, chip8->V[chip8->inst.Y]);
            break;

        case 0x2:
            // 0x8XY2: Set VX to VX and VY (bitwise)
            printf("Set V%X (0x%02X) to V%X and V%X (0x%02X) (bitwise)\n",
                chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.X,
                chip8->inst.Y, chip8->V[chip8->inst.Y]);
            break;

        case 0x3:
            // 0x8XY3: Set VX to VX xor VY
            printf("Set V%X (0x%02X) to V%X xor V%X (0x%02X)\n",
                chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.X,
                chip8->inst.Y, chip8->V[chip8->inst.Y]);
            break;

        case 0x4:
            // 0x8XY4: Add VY to VX, set VF
            printf("Add V%X (0x%02X) to V%X (0x%02X), set VF flag\n",
                chip8->inst.X, chip8->V[chip8->inst.X],
                chip8->inst.Y, chip8->V[chip8->inst.Y]);
            break;

        case 0x5:
            // 0x8XY5: VY is subtracted from VX, set VF
            printf("V%X (0x%02X) is subtracted from V%X (0x%02X), set VF flag\n",
                chip8->inst.Y, chip8->V[chip8->inst.Y],
                chip8->inst.X, chip8->V[chip8->inst.X]);
            break;

        case 0x6:
            // 0x8XY6: Stores the least significant bit of VX in VF
            //   and then shifts VX to the right by 1
            printf("Shift V%X to the right by 1, set VF flag\n", chip8->inst.X);
            break;

        case 0x7:
            // 0x8XY7: Set VX to VY minus VX, set VF
            printf("Set V%X to V%X (%02X) minus V%X (%02X), set VF\n",
                chip8->inst.X, chip8->inst.Y, chip8->V[chip8->inst.Y],
                chip8->inst.X, chip8->V[chip8->inst.X]);
            break;

        case 0xE:
            // 0x8XYE: Stores the most significant bit of VX in VF
            //  and then shifts VX to the left by 1
            printf("Shift V%X to the left by 1, set VF flag\n", chip8->inst.X);
            break;

        default:
            // Not implemented or bad opcode
            printf("Not implemented or bad opcode\n");
            break;
        }
        break;

    case 0x09:
        // 0x9XY0: Skip the next instruction if VX does not equal VY
        printf("Skip the next instruction if V%X (%02X) does not equal V%X (%02X)\n",
            chip8->inst.X, chip8->V[chip8->inst.X],
            chip8->inst.Y, chip8->V[chip8->inst.Y]);
        break;

    case 0x0A:
        // 0xANNN: Set I to the address NNN
        printf("Set I to NNN (0x%04X)\n", chip8->inst.NNN);
        break;

    case 0x0B:
        // 0xBNNN: Jump to the address NNN plus V0
        printf("Jump to the address NNN (0x%04X) plus V0 (0x%02X)\n", chip8->inst.NNN, chip8->V[0]);
        break;

    case 0x0C:
        // 0xCXNN: Set VX to the result of a bitwise and operation
        //  on a random number and NN
        printf("Set V%X to the result of a bitwise and"
            "operation on a random number and NN (0x02%X)\n", chip8->inst.X, chip8->inst.NN);
        break;

    case 0x0D:
        // 0xDXYN: Draw a sprite at coordinate (VX, VY)
        //  Read from memory location I.
        //  VF (Carry flag) is set if any screen pixels are set off
        printf("Draw N (%u) height sprite at coords "
            "V%X (0x%02X), V%X (0x%02X) from memory location I (0x%04X)\n",
            chip8->inst.N,
            chip8->inst.X, chip8->V[chip8->inst.X],
            chip8->inst.Y, chip8->V[chip8->inst.Y], chip8->I);
        break;

    case 0x0E:
        switch (chip8->inst.NN) {
        case 0x9E:
            // 0xEX9E: Skip the next instruction if the key stored in VX is pressed
            printf("Skip the next instruction if the key stored in V%X (0x%02X) is pressed\n",
                chip8->inst.X, chip8->V[chip8->inst.X]);
            break;

        case 0xA1:
            // 0xEXA1: Skips the next instruction if the key stored in VX is not pressed
            printf("Skip the next instruction if the key stored in V%X (0x%02X) is not pressed\n",
                chip8->inst.X, chip8->V[chip8->inst.X]);
            break;

        default:
            printf("Not implemented or invalid opcode\n");
            break; // Not implemented or invalid opcode
        }
        break;

    case 0x0F:
        switch (chip8->inst.NN) {
        case 0x07:
            // 0xFX07: Set VX to the value of the delay timer
            printf("Set V%X to the value of the delay timer (0x%02X)\n",
                chip8->inst.X, chip8->delay_timer);
            break;

        case 0x0A:
            // 0xFX0A: A key press is awaited, and then stored in VX (blocking operation)
            printf("A key press is awaited, and then stored in V%X (blocking operation)\n",
                chip8->inst.X);
            break;

        case 0x15:
            // 0xFX15: Set the delay timer to VX
            printf("Set the delay timer (0x%02X) to V%X\n",
                chip8->delay_timer, chip8->inst.X);
            break;

        case 0x18:
            // 0xFX18: Set the sound timer to VX
            printf("Set the sound timer (0x%02X) to V%X\n",
                chip8->sound_timer, chip8->inst.X);
            break;

        case 0x29:
            // 0xFX29: Set I to the location of the sprite for the character in VX.
            //   Characters 0-F (in hexadecimal) are represented by a 4x5 font
            printf("Set I to the location of the sprite for the character in V%X (0x%02X)\n",
                chip8->inst.X, chip8->V[chip8->inst.X]);
            break;

        case 0x33:
            // 0xFX33: Stores the BCD representation of VX,
            //   with the hundreds digit in memory at location in I, the tens
            //   digit at location I+1, and the ones digit at location I+2
            printf("Stores the BCD representation of V%X in memory at location I (0x%02X)\n",
                chip8->inst.X, chip8->I);
            break;

        case 0x55:
            // 0xFX55: Stores from V0 to VX (including VX) in memory, starting at address I.
            printf("Dump registers from V0 to V%X at memory location I (0x%02X)\n",
                chip8->inst.X, chip8->I);
            break;

        case 0x65:
            // 0xFX65: Stores from V0 to VX (including VX) in memory, starting at address I.
            printf("Load registers from V0 to V%X from memory location I (0x%02X)\n",
                chip8->inst.X, chip8->I);
            break;

        case 0x1E:
            // 0xFX1E: Add VX to I. VF is not affected
            printf("Add V%X to I (0x%04X)\n", chip8->inst.X, chip8->I);
            break;

        default:
            printf("Not implemented or invalid opcode\n");
            break; // Not implemented or invalid opcode
        }
        break;

    default:
        printf("Not implemented or bad opcode\n");
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
            // Pop last address from the stack
            //  and set PC to it
            chip8->PC = *--chip8->stack_ptr;
            break;

        default:
            // Not implemented
            //   or 0x0NNN: Calls machine code routine at address NNN
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

    case 0x03:
        // 0x3XNN: Skip the next instruction if VX equals NN
        if (chip8->V[chip8->inst.X] == chip8->inst.NN) {
            chip8->PC += 2;
        }
        break;

    case 0x04:
        // 0x4XNN: Skip the next instruction if VX does not equal NN
        if (chip8->V[chip8->inst.X] != chip8->inst.NN) {
            chip8->PC += 2;
        }
        break;

    case 0x05:
        // 0x5XY0: Skip the next instruction if VX equals VY
        if (chip8->V[chip8->inst.X] == chip8->V[chip8->inst.Y]) {
            chip8->PC += 2;
        }
        break;

    case 0x06:
        // 0x6XNN: Set VX to NN
        chip8->V[chip8->inst.X] = chip8->inst.NN;
        break;

    case 0x07:
        // 0x7XNN: Adds NN to VX (carry flag is not changed)
        chip8->V[chip8->inst.X] += chip8->inst.NN;
        break;

    case 0x08:
        switch (chip8->inst.N) {
        case 0x0:
            // 0x8XY0: Set VX to the value of VY
            chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y];
            break;

        case 0x1:
            // 0x8XY1: Set VX to VX or VY (bitwise)
            chip8->V[chip8->inst.X] |= chip8->V[chip8->inst.Y];
            break;

        case 0x2:
            // 0x8XY2: Set VX to VX and VY (bitwise)
            chip8->V[chip8->inst.X] &= chip8->V[chip8->inst.Y];
            break;

        case 0x3:
            // 0x8XY3: Set VX to VX xor VY
            chip8->V[chip8->inst.X] ^= chip8->V[chip8->inst.Y];
            break;

        case 0x4:
            // 0x8XY4: Add VY to VX, set VF
            if ((uint16_t)(chip8->V[chip8->inst.X] + chip8->V[chip8->inst.Y]) > 255) {
                chip8->V[0xF] = 1;
            }
            chip8->V[chip8->inst.X] += chip8->V[chip8->inst.Y];
            break;

        case 0x5:
            // 0x8XY5: VY is subtracted from VX, set VF
            chip8->V[0xF] = chip8->V[chip8->inst.Y] <= chip8->V[chip8->inst.X];

            chip8->V[chip8->inst.X] -= chip8->V[chip8->inst.Y];
            break;

        case 0x6:
            // 0x8XY6: Stores the least significant bit of VX in VF
            //   and then shifts VX to the right by 1
            //
            // CHIP-8 shifs the value in the register VY and stores the result in VX.
            //   The CHIP-48 and SCHIP implementations instead ignored VY, and simply shifted VX
            // TODO: Make this configurable
            chip8->V[0xF] = chip8->V[chip8->inst.X] & 1;
            chip8->V[chip8->inst.X] >>= 1;
            break;

        case 0x7:
            // 0x8XY7: Set VX to VY minus VX, set VF
            chip8->V[0xF] = chip8->V[chip8->inst.X] <= chip8->V[chip8->inst.Y];

            chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] - chip8->V[chip8->inst.X];
            break;

        case 0xE:
            // 0x8XYE: Stores the most significant bit of VX in VF
            //  and then shifts VX to the left by 1
            //
            // CHIP-8 shifs the value in the register VY and stores the result in VX.
            //   The CHIP-48 and SCHIP implementations instead ignored VY, and simply shifted VX
            // TODO: Make this configurable
            chip8->V[0xF] = (chip8->V[chip8->inst.X] & 0x80) >> 7;
            chip8->V[chip8->inst.X] <<= 1;
            break;

        default:
            // Not implemented or bad opcode
            break;
        }
        break;

    case 0x09:
        // 0x9XY0: Skip the next instruction if VX does not equal VY
        if (chip8->V[chip8->inst.X] != chip8->V[chip8->inst.Y]) {
            chip8->PC += 2;
        }
        break;

    case 0x0A:
        // 0xANNN: Set I to the address NNN
        chip8->I = chip8->inst.NNN;
        break;

    case 0x0B:
        // 0xBNNN: Jump to the address NNN plus V0
        chip8->PC = chip8->inst.NNN + chip8->V[0];
        break;

    case 0x0C:
        // 0xCXNN: Set VX to the result of a bitwise and operation
        //  on a random number and NN
        chip8->V[chip8->inst.X] = (rand() % 256) & chip8->inst.NN;
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

    case 0x0E:
        switch (chip8->inst.NN) {
        case 0x9E:
            // 0xEX9E: Skip the next instruction if the key stored in VX is pressed
            if (chip8->keypad[chip8->V[chip8->inst.X]]) {
                chip8->PC += 2;
            }
            break;

        case 0xA1:
            // 0xEXA1: Skips the next instruction if the key stored in VX is not pressed
            if (!chip8->keypad[chip8->V[chip8->inst.X]]) {
                chip8->PC += 2;
            }
            break;

        default:
            break; // Not implemented or invalid opcode
        }
        break;

    case 0x0F:
        switch (chip8->inst.NN) {
        case 0x07:
            // 0xFX07: Set VX to the value of the delay timer
            chip8->V[chip8->inst.X] = chip8->delay_timer;
            break;

        case 0x0A:
            // 0xFX0A: A key press is awaited, and then stored in VX (blocking operation)
            bool key_pressed = false;
            for (uint8_t i = 0; i < sizeof(chip8->keypad); ++i) {
                if (chip8->keypad[i]) {
                    chip8->V[chip8->inst.X] = i;
                    key_pressed = true;
                    break;
                }
            }

            // Keep running this instruction until a key is pressed
            if (!key_pressed) {
                chip8->PC -= 2;
            }
            break;

        case 0x15:
            // 0xFX15: Set the delay timer to VX
            chip8->delay_timer = chip8->V[chip8->inst.X];
            break;

        case 0x18:
            // 0xFX18: Set the sound timer to VX
            chip8->sound_timer = chip8->V[chip8->inst.X];
            break;

        case 0x1E:
            // 0xFX1E: Add VX to I. VF is not affected.
            // CHIP-8 interpreter for the Commodore Amiga sets VF to 1
            //   when there is a range overflow (I+VX>0xFFF)
            // TODO: Make setting VF configurable
            chip8->I += chip8->V[chip8->inst.X];
            break;

        case 0x29:
            // 0xFX29: Set I to the location of the sprite for the character in VX.
            //   Characters 0-F (in hexadecimal) are represented by a 4x5 font
            chip8->I = chip8->V[chip8->inst.X] * 5;
            break;

        case 0x33:
            // 0xFX33: Stores the BCD representation of VX,
            //   with the hundreds digit in memory at location in I, the tens
            //   digit at location I+1, and the ones digit at location I+2
            uint8_t bcd = chip8->V[chip8->inst.X];

            chip8->ram[chip8->I + 2] = bcd % 10;
            bcd /= 10;
            chip8->ram[chip8->I + 1] = bcd % 10;
            bcd /= 10;
            chip8->ram[chip8->I] = bcd;
            break;

        case 0x55:
            // 0xFX55: Stores from V0 to VX (including VX) in memory, starting at address I.
            //   CHIP-8 increments I, SCHIP does not
            // TODO: Make this configurable
            for (uint8_t i = 0; i <= chip8->inst.X; ++i) {
                chip8->ram[chip8->I + i] = chip8->V[i];
            }
            break;

        case 0x65:
            // 0xFX65: Stores from V0 to VX (including VX) in memory, starting at address I.
            //   CHIP-8 increments I, SCHIP does not
            // TODO: Make this configurable
            for (uint8_t i = 0; i <= chip8->inst.X; ++i) {
                chip8->V[i] = chip8->ram[chip8->I + i];
            }
            break;

        default:
            break; // Not implemented or invalid opcode
        }
        break;

    default:
        break; // Not implemented or invalid opcode
    }
}

