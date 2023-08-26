#ifndef _EMU_H_
#define _EMU_H_

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "chip.h"

#define WINDOW_SCALE 15

typedef enum EmulatorState
{
    STATE_RUNNING = 0,
    STATE_PAUSED,
    STATE_QUIT
} EmulatorState;

typedef struct Emulator
{
    SDL_Window* window;
    SDL_Renderer* renderer;

    EmulatorState state;
    Chip8 chip8;
} Emulator;

bool emu_init(Emulator* emu, const char* rom_file);
void emu_cleanup(const Emulator emu);
void emu_clear_screen(const Emulator emu);
void emu_update_screen(const Emulator emu);
void emu_handle_events(Emulator* emu);

#endif // _EMU_H_

