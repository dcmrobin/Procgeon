#ifndef GAMEAUDIO_H
#define GAMEAUDIO_H

#include <cstdint>
#include <SDL.h>
#include "Translation.h"

#define NUM_SFX 25
#define MAX_SFX_SIZE 30000  // ~0.68 sec at 44.1kHz
#define MAX_SIMULTANEOUS_SFX 4  // Number of sounds that can play at once

extern int ambientNoiseLevel;

// SFX will be stored as Mix_Chunk pointers
extern Mix_Chunk* sfxChunks[NUM_SFX];
extern const char* sfxFilenames[NUM_SFX];

// Play a sound effect
bool playRawSFX(int sfxIndex, float volume = 0.3f);

// Call this every frame to service the audio system (not needed for SDL_mixer)
// void serviceRawSFX(); // Remove this

void initAudio();
void freeSFX();
bool loadSFXtoRAM();

#endif