#include "audio.h"

bool audio_init(Audio* audio)
{
    audio->volume = AUDIO_VOLUME;
    audio->wave_freq = AUDIO_WAVE_FREQUENCY;

    audio->want = (SDL_AudioSpec){
        .freq = AUDIO_FREQUENCY,
        .format = AUDIO_FORMAT,
        .channels = AUDIO_CHANNELS,
        .samples = AUDIO_SAMPLES,
        .callback = audio_callback,
        .userdata = audio
    };

    audio->device = SDL_OpenAudioDevice(NULL, 0, &audio->want, &audio->have, 0);

    if (audio->device == 0) {
        SDL_Log("Could not open SDL audio device: %s", SDL_GetError());
        return false;
    }

    if (audio->want.format != audio->have.format ||
        audio->want.channels != audio->have.channels) {
        SDL_Log("Audio specifications not met");
        return false;
    }

    return true;
}

void audio_cleanup(Audio audio)
{
    SDL_CloseAudioDevice(audio.device);
}

void audio_callback(void* userdata, uint8_t* stream, int len)
{
    Audio* config = (Audio*)userdata;

    int16_t* audio_buffer = (int16_t*)stream;
    static uint32_t running_sample_index = 0;
    const int32_t square_wave_period = config->have.freq / config->wave_freq;
    const int32_t half_square_wave_period = square_wave_period / 2;

    // We are filling 2 bytes at a time, len is in bytes
    //   so divide by 2
    for (int i = 0; i < len / 2; ++i) {
        audio_buffer[i] = ((running_sample_index++ / half_square_wave_period) % 2) ?
            config->volume :
            -config->volume;
    }
}

