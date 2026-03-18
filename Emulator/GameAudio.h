#ifndef GAMEAUDIO_H
#define GAMEAUDIO_H

#include "Common.h"

// ─────────────────────────────────────────────────────────────────────────────
// GameAudio.h
//
// Audio system interface. The actual SDL2-backed implementation lives in
// GameAudio.cpp.  Translation.h provides the AudioPlaySdWav / AudioMixer4
// stubs, so this header just declares the game-level API.
// ─────────────────────────────────────────────────────────────────────────────

// ── Hardware audio objects (defined in GameAudio.cpp) ────────────────────
extern AudioPlayQueue       queue[MAX_SIMULTANEOUS_SFX];
extern AudioMixer4          mixer1;
extern AudioMixer4          mixer2;
extern AudioMixer4          musicMixer;
extern AudioOutputI2S       audioOutput;
extern AudioControlSGTL5000 sgtl5000_1;

// ── SFX asset storage ────────────────────────────────────────────────────
extern uint8_t*     sfxData[NUM_SFX];
extern size_t       sfxLength[NUM_SFX];
extern const char*  sfxFilenames[NUM_SFX];
extern RawSFXPlayback activeSFX[MAX_SIMULTANEOUS_SFX];

// ── Functions ─────────────────────────────────────────────────────────────
void initAudio();
bool loadSFXtoRAM();
void freeSFX();
void serviceRawSFX();

bool playRawSFX(int sfxIndex);
bool playRawSFX3D(int sfxIndex, float soundX, float soundY);
void setJukeboxVolume(float v);

#endif // GAMEAUDIO_H