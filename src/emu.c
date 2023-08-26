#include "emu.h"

#include <stdio.h>

bool emu_init(Emulator* emu, const char* rom_file)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("Could not initialize SDL: %s", SDL_GetError());
        return false;
    }

    emu->window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH * WINDOW_SCALE, WINDOW_HEIGHT * WINDOW_SCALE,
        0);

    if (!emu->window) {
        SDL_Log("Could not initialize SDL window: %s", SDL_GetError());
        return false;
    }

    emu->renderer = SDL_CreateRenderer(emu->window, -1, SDL_RENDERER_ACCELERATED);

    if (!emu->renderer) {
        SDL_Log("Could not initialize SDL renderer: %s", SDL_GetError());
        return false;
    }

    emu->state = STATE_RUNNING;

    if (!chip8_init(&emu->chip8, rom_file)) {
        fprintf(stderr, "ERROR: Could not initialize CHIP-8\n");
        return false;
    }

    return true;
}

void emu_cleanup(const Emulator emu)
{
    SDL_DestroyRenderer(emu.renderer);
    SDL_DestroyWindow(emu.window);
    SDL_Quit();
}

void emu_clear_screen(const Emulator emu)
{
    const uint8_t r = (BACKGROUND_COLOR >> 24) & 0xFF;
    const uint8_t g = (BACKGROUND_COLOR >> 16) & 0xFF;
    const uint8_t b = (BACKGROUND_COLOR >>  8) & 0xFF;
    const uint8_t a = (BACKGROUND_COLOR >>  0) & 0xFF;

    SDL_SetRenderDrawColor(emu.renderer, r, g, b, a);
    SDL_RenderClear(emu.renderer);
}

void emu_update_screen(const Emulator emu)
{
    SDL_Rect rect = { .x = 0, .y = 0, .w = WINDOW_SCALE, .h = WINDOW_SCALE };

    const uint8_t fg_r = (FOREGROUND_COLOR >> 24) & 0xFF;
    const uint8_t fg_g = (FOREGROUND_COLOR >> 16) & 0xFF;
    const uint8_t fg_b = (FOREGROUND_COLOR >>  8) & 0xFF;
    const uint8_t fg_a = (FOREGROUND_COLOR >>  0) & 0xFF;

    const uint8_t bg_r = (BACKGROUND_COLOR >> 24) & 0xFF;
    const uint8_t bg_g = (BACKGROUND_COLOR >> 16) & 0xFF;
    const uint8_t bg_b = (BACKGROUND_COLOR >>  8) & 0xFF;
    const uint8_t bg_a = (BACKGROUND_COLOR >>  0) & 0xFF;

    // Loop through display pixels and draw a rectangle per pixel
    for (uint32_t i = 0; i < sizeof(emu.chip8.display); ++i) {
        // Translate 1D index to 2D X / Y coordinates
        rect.x = (i % WINDOW_WIDTH) * WINDOW_SCALE;
        rect.y = (i / WINDOW_WIDTH) * WINDOW_SCALE;

        if (emu.chip8.display[i]) {
            // If pixel is on, draw foreground color
            SDL_SetRenderDrawColor(emu.renderer, fg_r, fg_g, fg_b, fg_a);
            SDL_RenderFillRect(emu.renderer, &rect);

            // Draw pixel outlines
            SDL_SetRenderDrawColor(emu.renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderDrawRect(emu.renderer, &rect);
        } else {
            // If pixel is off, draw background color
            SDL_SetRenderDrawColor(emu.renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderFillRect(emu.renderer, &rect);
        }
    }

    SDL_RenderPresent(emu.renderer);
}

void emu_handle_events(Emulator* emu)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            emu->state = STATE_QUIT;
            return;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                emu->state = STATE_QUIT;
                return;
            
            case SDLK_SPACE:
                if (emu->state == STATE_RUNNING) {
                    emu->state = STATE_PAUSED;
                    puts("INFO: Emulator paused");
                } else {
                    emu->state = STATE_RUNNING;
                    puts("INFO: Emulator resumed");
                }
                return;

            default:
                break;
            }
            break;

        case SDL_KEYUP:
            break;

        default:
            break;
        }
    }
}

