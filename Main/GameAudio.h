#ifndef GAMEAUDIO_H
#define GAMEAUDIO_H

#include <Arduino.h>
#include <Audio.h>

#define NUM_SFX 24
#define MAX_SFX_SIZE 30000  // ~0.68 sec at 44.1kHz

// Declare audio objects (defined in .cpp)
extern AudioPlaySdWav      playWav1;
extern AudioPlayQueue      queue1;
extern AudioMixer4         mixer1;
extern AudioOutputI2S      audioOutput;

extern AudioConnection     patchCord1;
extern AudioConnection     patchCord2;
extern AudioConnection     patchCord3;
extern AudioConnection     patchCord4;
extern AudioControlSGTL5000 sgtl5000_1;

extern uint8_t* sfxData[NUM_SFX];
extern size_t sfxLength[NUM_SFX];
extern const char* sfxFilenames[NUM_SFX];

struct RawSFXPlayback {
    const int16_t* data = nullptr;
    size_t samplesTotal = 0;
    size_t samplesPlayed = 0;
    bool isPlaying = false;
};

extern RawSFXPlayback currentSFX;

void playRawSFX(const uint8_t* data, size_t length);  // initiate playback
void serviceRawSFX();  // call this every frame

void freeSFX();
bool loadSFXtoRAM();

#endif