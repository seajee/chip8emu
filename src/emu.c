#include "emu.h"

#include <stdio.h>

bool emu_init(Emulator* emu, const char* rom_file)
{
    // Initialize SDL
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

    // Initialize emulator components
    if (!chip8_init(&emu->chip8, rom_file)) {
        fprintf(stderr, "ERROR: Could not initialize CHIP-8\n");
        return false;
    }

    if (!audio_init(&emu->audio)) {
        fprintf(stderr, "ERROR: Could not initialize audio\n");
        return false;
    }

    emu->state = STATE_RUNNING;

    return true;
}

void emu_cleanup(const Emulator emu)
{
    SDL_DestroyRenderer(emu.renderer);
    SDL_DestroyWindow(emu.window);
    audio_cleanup(emu.audio);
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
    // TODO: Make specific functions to contain SDL graphics
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
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                emu->state = STATE_QUIT;
                break;
            
            case SDLK_SPACE:
                if (emu->state == STATE_RUNNING) {
                    emu->state = STATE_PAUSED;
                    puts("INFO: Emulator paused");
                } else {
                    emu->state = STATE_RUNNING;
                    puts("INFO: Emulator resumed");
                }
                break;
            
            // CHIP-8 Keypad | QWERTY Keyboard
            // 123C          | 1234
            // 456D          | QWER
            // 789E          | ASDF
            // A0BF          | ZXCV
            case SDLK_1: emu->chip8.keypad[0x1] = true; break;
            case SDLK_2: emu->chip8.keypad[0x2] = true; break;
            case SDLK_3: emu->chip8.keypad[0x3] = true; break;
            case SDLK_4: emu->chip8.keypad[0xC] = true; break;

            case SDLK_q: emu->chip8.keypad[0x4] = true; break;
            case SDLK_w: emu->chip8.keypad[0x5] = true; break;
            case SDLK_e: emu->chip8.keypad[0x6] = true; break;
            case SDLK_r: emu->chip8.keypad[0xD] = true; break;

            case SDLK_a: emu->chip8.keypad[0x7] = true; break;
            case SDLK_s: emu->chip8.keypad[0x8] = true; break;
            case SDLK_d: emu->chip8.keypad[0x9] = true; break;
            case SDLK_f: emu->chip8.keypad[0xE] = true; break;

            case SDLK_z: emu->chip8.keypad[0xA] = true; break;
            case SDLK_x: emu->chip8.keypad[0x0] = true; break;
            case SDLK_c: emu->chip8.keypad[0xB] = true; break;
            case SDLK_v: emu->chip8.keypad[0xF] = true; break;

            default:
                break;
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_1: emu->chip8.keypad[0x1] = false; break;
            case SDLK_2: emu->chip8.keypad[0x2] = false; break;
            case SDLK_3: emu->chip8.keypad[0x3] = false; break;
            case SDLK_4: emu->chip8.keypad[0xC] = false; break;

            case SDLK_q: emu->chip8.keypad[0x4] = false; break;
            case SDLK_w: emu->chip8.keypad[0x5] = false; break;
            case SDLK_e: emu->chip8.keypad[0x6] = false; break;
            case SDLK_r: emu->chip8.keypad[0xD] = false; break;

            case SDLK_a: emu->chip8.keypad[0x7] = false; break;
            case SDLK_s: emu->chip8.keypad[0x8] = false; break;
            case SDLK_d: emu->chip8.keypad[0x9] = false; break;
            case SDLK_f: emu->chip8.keypad[0xE] = false; break;

            case SDLK_z: emu->chip8.keypad[0xA] = false; break;
            case SDLK_x: emu->chip8.keypad[0x0] = false; break;
            case SDLK_c: emu->chip8.keypad[0xB] = false; break;
            case SDLK_v: emu->chip8.keypad[0xF] = false; break;

            default:
                break;
            }

            break;

        default:
            break;
        }
    }
}

void emu_update_timers(Emulator* emu)
{
    Chip8* chip8 = &emu->chip8;

    if (chip8->delay_timer > 0) {
        chip8->delay_timer--;
    }

    if (chip8->sound_timer > 0) {
        chip8->sound_timer--;
        SDL_PauseAudioDevice(emu->audio.device, 0); // Play sound
    } else {
        SDL_PauseAudioDevice(emu->audio.device, 1); // Pause sound
    }
}

