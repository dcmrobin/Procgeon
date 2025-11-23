#ifndef GAMEAUDIO_H
#define GAMEAUDIO_H

#include <Arduino.h>
#include <Audio.h>

#define NUM_SFX 25
#define MAX_SFX_SIZE 30000  // ~0.68 sec at 44.1kHz
#define MAX_SIMULTANEOUS_SFX 8  // Number of sounds that can play at once
#define MAX_AUDIO_DISTANCE 20  // Maximum distance for sound to be heard
#define MIN_AUDIO_VOLUME 0.01f   // Minimum volume before sound cuts out

// Declare audio objects (defined in .cpp)
extern AudioPlayQueue      queue[MAX_SIMULTANEOUS_SFX];
extern AudioMixer4         mixer1;  // For queues 0-3
extern AudioMixer4         mixer2;  // For queues 4-7
extern AudioMixer4         musicMixer;
extern AudioOutputI2S      audioOutput;
extern AudioPlaySdWav      playWav1;
extern AudioPlaySdWav      playWav2;  // Jukebox music player
//extern AudioAmplifier      amp1;

extern int ambientNoiseLevel;
extern int masterVolume; // 1..10
extern float jukeboxVolume; // 0.0 .. 1.0
void setJukeboxVolume(float v);

// Audio connections will be defined in the cpp file

extern AudioControlSGTL5000 sgtl5000_1;

extern uint8_t* sfxData[NUM_SFX];
extern size_t sfxLength[NUM_SFX];
extern const char* sfxFilenames[NUM_SFX];

struct RawSFXPlayback {
    const int16_t* data = nullptr;
    size_t samplesTotal = 0;
    size_t samplesPlayed = 0;
    bool isPlaying = false;
    float volume = 1.0f;  // Volume for this sound (0.0 to 1.0)
};

// Array of currently playing sound effects
extern RawSFXPlayback activeSFX[MAX_SIMULTANEOUS_SFX];

// Play a sound effect
bool playRawSFX(int sfxIndex);

// Play a sound effect with 3D positioning (x, y coordinates)
bool playRawSFX3D(int sfxIndex, float soundX, float soundY);

// Call this every frame to service the audio system
void serviceRawSFX();

void initAudio();
void freeSFX();
//bool loadSFXtoRAM();

// Utility function to play specific sound effects by name
//inline bool playSFX_PlayerHurt()     { return playRawSFX(0); }
//inline bool playSFX_PlayerShoot()    { return playRawSFX(1); }
//inline bool playSFX_PlayerUse()      { return playRawSFX(2); }
// Add more convenience functions as needed...

#endif