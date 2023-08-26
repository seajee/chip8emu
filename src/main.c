#include <stdio.h>

#include "emu.h"
#include "chip.h"

int main(int argc, char** argv)
{
    if (argc <= 1) {
        fprintf(stderr, "Usage: chip8emu <rom file>\n");
        return EXIT_FAILURE;
    }

    const char* rom_path = argv[1];

    Emulator emu;

    if (!emu_init(&emu, rom_path)) {
        fprintf(stderr, "ERROR: Could not initialize emulator\n");
        return EXIT_FAILURE;
    }

    while (emu.state != STATE_QUIT) {
        emu_handle_events(&emu);

        if (emu.state == STATE_PAUSED) {
            continue;
        }

        // Get time before running instructions
        uint64_t start_timer = SDL_GetPerformanceCounter();

        // Emulate CHIP-8 instructions for this frame (60 Hz)
        for (uint32_t i = 0; i < CHIP_INST_PER_SECOND / FPS; ++i) {
            chip8_execute(&emu.chip8);
        }

        // Get time after running instructions
        uint64_t end_timer = SDL_GetPerformanceCounter();

        // Delay for 60 fps
        const double time_elapsed = (double)((end_timer - start_timer) * 1000) / SDL_GetPerformanceFrequency();
        const double delay = (1000 / FPS) > time_elapsed ? (1000 / FPS) - time_elapsed : 0;
        SDL_Delay(delay);

        // Clear and update the emulator screen
        emu_clear_screen(emu);
        emu_update_screen(emu);

        // Update CHIP-8 timers
        chip8_update_timers(&emu.chip8);
    }

    emu_cleanup(emu);

    return EXIT_SUCCESS;
}

