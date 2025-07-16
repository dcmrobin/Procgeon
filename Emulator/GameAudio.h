#ifndef GAMEAUDIO_H
#define GAMEAUDIO_H

#include <cstdint>
#include <SDL.h>

#define NUM_SFX 24
#define MAX_SFX_SIZE 30000
#define MAX_SIMULTANEOUS_SFX 4

// Remove Teensy audio objects
extern uint8_t* sfxData[NUM_SFX];
extern size_t sfxLength[NUM_SFX];
extern const char* sfxFilenames[NUM_SFX];

struct RawSFXPlayback {
    const int16_t* data = nullptr;
    size_t samplesTotal = 0;
    size_t samplesPlayed = 0;
    bool isPlaying = false;
    float volume = 1.0f;

    int index = -1;
    int position = 0;
    int length = 0;
    bool playing = false;

    RawSFXPlayback() : index(-1), position(0), length(0), volume(1.0f), playing(false) {}
    RawSFXPlayback(int idx, int pos, int len, float vol, bool play) : index(idx), position(pos), length(len), volume(vol), playing(play) {}
};

extern RawSFXPlayback activeSFX[MAX_SIMULTANEOUS_SFX];
extern float globalVolume;  // Master volume control

bool playRawSFX(int sfxIndex);
void initAudio();
void closeAudio();  // New cleanup function
void freeSFX();
bool loadSFXtoRAM();

// Utility functions
inline bool playSFX_PlayerHurt()     { return playRawSFX(0); }
inline bool playSFX_PlayerShoot()    { return playRawSFX(1); }
inline bool playSFX_PlayerUse()      { return playRawSFX(2); }

#endif