#include "GameAudio.h"
#include <algorithm>
#include <SDL.h>
#include <SDL2/SDL_mixer.h>
#include <cstring>
#include <cstdio>
#include <vector>
#include <algorithm>

// Define the audio system objects
//AudioPlayQueue      queue[MAX_SIMULTANEOUS_SFX];
//AudioMixer4         mixer1;
//AudioMixer4         musicMixer; // New mixer for music
//AudioOutputI2S      audioOutput;
//AudioPlaySdWav      playWav1;

// Create audio connections
AudioConnection     patchCord1(queue[0], 0, mixer1, 0);
AudioConnection     patchCord2(queue[1], 0, mixer1, 1);
AudioConnection     patchCord3(queue[2], 0, mixer1, 2);
AudioConnection     patchCord4(queue[3], 0, mixer1, 3);
AudioConnection     patchCord5(mixer1, 0, musicMixer, 0); // SFX to musicMixer
AudioConnection     patchCord6(playWav1, 0, musicMixer, 1); // WAV to musicMixer
AudioConnection     patchCord7(musicMixer, 0, audioOutput, 0);
AudioConnection     patchCord8(musicMixer, 0, audioOutput, 1);
//AudioControlSGTL5000 sgtl5000_1;

int ambientNoiseLevel = 0;

// RAM-loaded sound effect storage
uint8_t* sfxData[NUM_SFX] = { nullptr };
size_t sfxLength[NUM_SFX] = { 0 };

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
  "bullet_impactEnemy.raw",// 23
  "succubus_hey.raw"//        24
};

// Array of currently playing sound effects
RawSFXPlayback activeSFX[MAX_SIMULTANEOUS_SFX];

// Initialize the audio system
void initAudio() {
    // Enable the audio shield
    AudioMemory(30);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);
    // Set mixer levels for each channel
    mixer1.gain(0, 0.5);
    mixer1.gain(1, 0.5);
    mixer1.gain(2, 0.5);
    mixer1.gain(3, 0.5);
    musicMixer.gain(0, 1.0); // SFX
    musicMixer.gain(1, 0.2); // WAV music
    musicMixer.gain(2, 0.0);
    musicMixer.gain(3, 0.0);
}

bool playRawSFX(int sfxIndex) {
    ambientNoiseLevel++;
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX) return false;
    if (!sfxData[sfxIndex]) return false;
    
    // Find an available playback slot
    int slot = -1;
    for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
        if (!activeSFX[i].isPlaying) {
            slot = i;
            break;
        }
    }
    
    // If all slots are in use, return false or optionally replace the oldest sound
    if (slot == -1) {
        // Option 1: Fail to play the new sound
        return false;
        
        // Option 2: Replace the sound that's played the most
        /*
        int maxPlayedSamples = 0;
        for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
            if (activeSFX[i].samplesPlayed > maxPlayedSamples) {
                maxPlayedSamples = activeSFX[i].samplesPlayed;
                slot = i;
            }
        }
        */
    }
    
    // Initialize the sound in the chosen slot
    activeSFX[slot].data = (const int16_t*)sfxData[sfxIndex];
    activeSFX[slot].samplesTotal = sfxLength[sfxIndex] / 2;
    activeSFX[slot].samplesPlayed = 0;
    activeSFX[slot].isPlaying = true;
    activeSFX[slot].volume = constrain(1, 0.0f, 1.0f);
    
    return true;
}

void serviceRawSFX() {
    // Process each active sound effect
    for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
        RawSFXPlayback& sfx = activeSFX[i];
        
        if (!sfx.isPlaying) continue;
        if (!queue[i].available()) continue;

        int16_t* block = queue[i].getBuffer();
        if (!block) continue;

        size_t remaining = sfx.samplesTotal - sfx.samplesPlayed;
        size_t samplesToCopy = min(remaining, static_cast<size_t>(AUDIO_BLOCK_SAMPLES));

        // Copy and apply volume
        if (sfx.volume == 1.0f) {
            // At full volume, just copy
            memcpy(block, sfx.data + sfx.samplesPlayed, samplesToCopy * 2);
        } else {
            // Apply volume scaling
            for (size_t j = 0; j < samplesToCopy; j++) {
                block[j] = sfx.data[sfx.samplesPlayed + j] * sfx.volume;
            }
        }

        // Clear the rest of the buffer if needed
        if (samplesToCopy < AUDIO_BLOCK_SAMPLES) {
            memset(((uint8_t*)block) + samplesToCopy * 2, 0, (AUDIO_BLOCK_SAMPLES - samplesToCopy) * 2);
        }

        queue[i].playBuffer();
        sfx.samplesPlayed += samplesToCopy;

        if (sfx.samplesPlayed >= sfx.samplesTotal) {
            sfx.isPlaying = false;
        }
    }
}

void freeSFX() {
    for (int i = 0; i < NUM_SFX; i++) {
        if (sfxData[i]) {
            free(sfxData[i]);
            sfxData[i] = nullptr;
        }
    }
}

bool loadSFXtoRAM() {
    for (int i = 0; i < NUM_SFX; i++) {
        File f = SD::open(sfxFilenames[i]);
        if (!f) {
            Serial.printf("Failed to open %s\n", sfxFilenames[i]);
            return false;
        }

        size_t len = f.size();
        if (len > MAX_SFX_SIZE) len = MAX_SFX_SIZE;

        sfxData[i] = (uint8_t*)malloc(len);
        if (!sfxData[i]) {
            Serial.printf("Failed to allocate memory for %s\n", sfxFilenames[i]);
            f.close();
            return false;
        }

        f.read(sfxData[i], len);
        sfxLength[i] = len;
        f.close();
    }
    return true;
}