#include "GameAudio.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <vector>
#include <algorithm>

int ambientNoiseLevel = 0;

// SFX storage as Mix_Chunk pointers
Mix_Chunk* sfxChunks[NUM_SFX] = { nullptr };

const char* sfxFilenames[NUM_SFX] = {
  "player_hurt.wav",//        0
  "player_shoot.wav",//       1
  "player_use.wav",//         2
  "player_pickup.wav",//      3
  "player_footstep.wav",//    4 IMPLEMENT
  "player_eat.wav",//         5
  "player_drink.wav",//       6
  "menu_select.wav",//        7
  "menu_scroll.wav",//        8
  "menu_pause.wav",//         9
  "menu_gameOver.wav",//      10
  "level_end.wav",//          11
  "inventory_open.wav",//     12
  "inventory_close.wav",//    13
  "enemy_teleport.wav",//     14
  "damsel_putDown.wav",//     15
  "damsel_passive.wav",//     16
  "damsel_hurt.wav",//        17
  "damsel_good.wav",//        18
  "damsel_footstep.wav",//    19 IMPLEMENT
  "damsel_carry.wav",//       20
  "damsel_annoying.wav",//    21
  "bullet_impactWall.wav",//  22
  "bullet_impactEnemy.wav",// 23
  "succubus_hey.wav"//        24
};

// Initialize the audio system
void initAudio() {
    // SDL_mixer is already initialized in main.cpp via initSDL2Audio()
    // No need for Teensy audio emulation
}

bool playRawSFX(int sfxIndex, float volume) {
    ambientNoiseLevel++;
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX) return false;
    if (!sfxChunks[sfxIndex]) return false;
    
    // Play the sound using SDL_mixer
    int channel = Mix_PlayChannel(-1, sfxChunks[sfxIndex], 0);
    if (channel == -1) {
        return false;
    }
    
    // Set volume (0-128 scale)
    int mixVolume = static_cast<int>(volume * 128);
    mixVolume = constrain(mixVolume, 0, 128);
    Mix_Volume(channel, mixVolume);
    
    return true;
}

// Remove serviceRawSFX since SDL_mixer handles playback automatically
/*
void serviceRawSFX() {
    // No longer needed - SDL_mixer handles playback
}
*/

void freeSFX() {
    for (int i = 0; i < NUM_SFX; i++) {
        if (sfxChunks[i]) {
            Mix_FreeChunk(sfxChunks[i]);
            sfxChunks[i] = nullptr;
        }
    }
}

bool loadSFXtoRAM() {
    for (int i = 0; i < NUM_SFX; i++) {
        // Try different paths
        const char* paths[] = {
            sfxFilenames[i],
            ("Audio/" + std::string(sfxFilenames[i])).c_str(),
            ("./Audio/" + std::string(sfxFilenames[i])).c_str(),
            nullptr
        };
        
        bool loaded = false;
        for (int p = 0; paths[p] != nullptr; p++) {
            sfxChunks[i] = Mix_LoadWAV(paths[p]);
            if (sfxChunks[i]) {
                printf("Loaded SFX: %s\n", paths[p]);
                loaded = true;
                break;
            }
        }
        
        if (!loaded) {
            printf("Failed to load SFX: %s\n", sfxFilenames[i]);
            return false;
        }
    }
    return true;
}