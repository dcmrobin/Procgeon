#include "GameAudio.h"
#include <SDL.h>
#include <cstring>
#include <cstdio>
#include <vector>
#include <algorithm>

uint8_t* sfxData[NUM_SFX] = { nullptr };
size_t sfxLength[NUM_SFX] = { 0 };
RawSFXPlayback activeSFX[MAX_SIMULTANEOUS_SFX];
float globalVolume = 0.125f;  // Default master volume (0.5 * 0.25 from original)

const char* sfxFilenames[NUM_SFX] = {
    "player_hurt.raw",//        0
    "player_shoot.raw",//       1
    "player_use.raw",//         2
    "player_pickup.raw",//      3
    "player_footstep.raw",//    4 IMPLEMENT
    "player_eat.raw",//         5
    "player_drink.raw",//       6
    "menu_select.raw",//        7
    "menu_scroll.raw",//        8
    "menu_pause.raw",//         9
    "menu_gameOver.raw",//      10
    "level_end.raw",//          11
    "inventory_open.raw",//     12
    "inventory_close.raw",//    13
    "enemy_teleport.raw",//     14
    "damsel_putDown.raw",//     15
    "damsel_passive.raw",//     16
    "damsel_hurt.raw",//        17
    "damsel_good.raw",//        18
    "damsel_footstep.raw",//    19 IMPLEMENT
    "damsel_carry.raw",//       20
    "damsel_annoying.raw",//    21
    "bullet_impactWall.raw",//  22
    "bullet_impactEnemy.raw"//  23
};

std::vector<float> mixBuffer; // Global mixing buffer

void audioCallback(void* userdata, Uint8* stream, int len) {
    int16_t* output = reinterpret_cast<int16_t*>(stream);
    const int numSamples = len / sizeof(int16_t);
    
    // Clear output buffer
    std::memset(stream, 0, len);

    // Temporary mixing buffer
    mixBuffer.resize(numSamples);
    std::fill(mixBuffer.begin(), mixBuffer.end(), 0.0f);
    std::fill(mixBuffer.begin(), mixBuffer.end(), 0.0f);

    // Mix active sounds
    for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
        if (!activeSFX[i].isPlaying) continue;

        RawSFXPlayback& sfx = activeSFX[i];
        const int16_t* data = sfx.data;
        const size_t samplesToMix = std::min(
            sfx.samplesTotal - sfx.samplesPlayed,
            static_cast<size_t>(numSamples / 2)  // Mono->stereo conversion
        );

        // Mix mono to stereo with volume
        for (size_t j = 0; j < samplesToMix; j++) {
            const float sample = data[sfx.samplesPlayed + j] * sfx.volume;
            const int idx = j * 2;  // Stereo position
            mixBuffer[idx] += sample;
            mixBuffer[idx + 1] += sample;
        }

        sfx.samplesPlayed += samplesToMix;
        if (sfx.samplesPlayed >= sfx.samplesTotal) {
            sfx.isPlaying = false;
        }
    }

    // Apply global volume and convert to int16
    for (int i = 0; i < numSamples; i++) {
        float sample = mixBuffer[i] * globalVolume;
        sample = std::clamp(sample, -32768.0f, 32767.0f);
        output[i] = static_cast<int16_t>(sample);
    }
}

void initAudio() {
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);
    
    desired.freq = 44100;
    desired.format = AUDIO_S16;
    desired.channels = 2;  // Stereo
    desired.samples = 512; // Buffer size
    desired.callback = audioCallback;

    if (SDL_OpenAudio(&desired, &obtained) < 0) {
        SDL_Log("Audio init failed: %s", SDL_GetError());
        return;
    }
    SDL_PauseAudio(0);  // Start playback
}

void closeAudio() {
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool playRawSFX(int sfxIndex) {
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX || !sfxData[sfxIndex]) 
        return false;

    SDL_LockAudio();  // Thread-safe access

    // Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
        if (!activeSFX[i].isPlaying) {
            slot = i;
            break;
        }
    }

    if (slot != -1) {
        activeSFX[slot] = {
            .data = reinterpret_cast<int16_t*>(sfxData[sfxIndex]),
            .samplesTotal = sfxLength[sfxIndex] / sizeof(int16_t),
            .samplesPlayed = 0,
            .isPlaying = true,
            .volume = 1.0f
        };
    }

    SDL_UnlockAudio();
    return slot != -1;
}

bool loadSFXtoRAM() {
    for (int i = 0; i < NUM_SFX; i++) {
        SDL_RWops* file = SDL_RWFromFile(sfxFilenames[i], "rb");
        if (!file) {
            SDL_Log("Failed to open %s: %s", sfxFilenames[i], SDL_GetError());
            return false;
        }

        const size_t len = std::min(
            static_cast<size_t>(SDL_RWsize(file)),
            static_cast<size_t>(MAX_SFX_SIZE)
        );
        
        sfxData[i] = static_cast<uint8_t*>(malloc(len));
        if (!sfxData[i]) {
            SDL_RWclose(file);
            return false;
        }

        SDL_RWread(file, sfxData[i], len, 1);
        sfxLength[i] = len;
        SDL_RWclose(file);
    }
    return true;
}

void freeSFX() {
    SDL_LockAudio();
    for (int i = 0; i < NUM_SFX; i++) {
        free(sfxData[i]);
        sfxData[i] = nullptr;
        sfxLength[i] = 0;
    }
    SDL_UnlockAudio();
}