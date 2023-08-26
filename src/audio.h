#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <SDL2/SDL.h>
#include <stdbool.h>

#define AUDIO_FREQUENCY 44100
#define AUDIO_FORMAT AUDIO_S16LSB
#define AUDIO_CHANNELS 1
#define AUDIO_SAMPLES 512

#define AUDIO_VOLUME 2000 // INT16
#define AUDIO_WAVE_FREQUENCY 440

typedef struct Audio {
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID device;

    int16_t volume;
    uint32_t wave_freq;
} Audio;

bool audio_init(Audio* audio);
void audio_cleanup(Audio audio);
void audio_callback(void* userdata, uint8_t* stream, int len);

#endif // _AUDIO_H_

