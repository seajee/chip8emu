#include <stdio.h>

#include "emu.h"

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
        SDL_Delay(16);

        if (emu.state == STATE_PAUSED) {
            continue;
        }

        chip8_execute(&emu.chip8);

        emu_clear_screen(emu);
        emu_update_screen(emu);
    }

    emu_cleanup(emu);

    return EXIT_SUCCESS;
}

