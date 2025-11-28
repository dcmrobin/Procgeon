#include "GameAudio.h"
#include "Translation.h"
#include "Player.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <vector>
#include <algorithm>

// Define the audio system objects (as stubs for SDL2)
AudioPlayQueue      queue[MAX_SIMULTANEOUS_SFX];
AudioMixer4         mixer1;
AudioMixer4         mixer2;
AudioMixer4         musicMixer;
AudioOutputI2S      audioOutput;
AudioControlSGTL5000 sgtl5000_1;

// Audio connections (stubs)
AudioConnection     patchCord1(queue[0], 0, mixer1, 0);
AudioConnection     patchCord2(queue[1], 0, mixer1, 1);
AudioConnection     patchCord3(queue[2], 0, mixer1, 2);
AudioConnection     patchCord4(queue[3], 0, mixer1, 3);
AudioConnection     patchCord5(queue[4], 0, mixer2, 0);
AudioConnection     patchCord6(queue[5], 0, mixer2, 1);
AudioConnection     patchCord7(queue[6], 0, mixer2, 2);
AudioConnection     patchCord8(queue[7], 0, mixer2, 3);
AudioConnection     patchCord9(mixer1, 0, musicMixer, 0);
AudioConnection     patchCord10(mixer2, 0, musicMixer, 1);
AudioConnection     patchCord11(playWav1, 0, musicMixer, 2);
AudioConnection     patchCord12(playWav2, 0, musicMixer, 3);
AudioConnection     patchCord13(musicMixer, 0, audioOutput, 0);
AudioConnection     patchCord14(musicMixer, 0, audioOutput, 1);

int ambientNoiseLevel = 0;
int masterVolume = 10;
float jukeboxVolume = 0.0f;

// RAM-loaded sound effect storage
uint8_t* sfxData[NUM_SFX] = { nullptr };
size_t sfxLength[NUM_SFX] = { 0 };

// SDL2 audio storage
Mix_Chunk* sfxChunks[NUM_SFX] = { nullptr };
std::vector<int> activeChannels;

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

// Array of currently playing sound effects
RawSFXPlayback activeSFX[MAX_SIMULTANEOUS_SFX];

// Initialize the audio system with SDL2
void initAudio() {
    // Initialize SDL2 audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        Serial.printf("SDL audio init failed: %s\n", SDL_GetError());
        return;
    }
    
    // Initialize SDL2_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        Serial.printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return;
    }
    
    // Allocate mixing channels
    Mix_AllocateChannels(16);
    
    // Set master volume
    float vol = constrain(masterVolume / 10.0f, 0.0f, 1.0f);
    Mix_Volume(-1, static_cast<int>(vol * MIX_MAX_VOLUME));
    
    // Pre-load all sound effects
    if (!loadSFXtoRAM()) {
        Serial.println("Failed to load SFX");
    } else {
        Serial.println("SFX loaded successfully");
    }
    
    Serial.println("Audio initialized with SDL2");
}

void setJukeboxVolume(float v) {
    jukeboxVolume = constrain(v, 0.0f, 0.23f);
    float masterVol = masterVolume / 10.0f;
    // SDL2 volume control for music
    if (playWav2.isPlaying()) {
        playWav2.volume(jukeboxVolume * masterVol);
    }
}

bool playRawSFX(int sfxIndex) {
    ambientNoiseLevel++;
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX) {
        Serial.printf("Invalid SFX index: %d\n", sfxIndex);
        return false;
    }
    
    // Use SDL2 mixer to play the sound
    if (sfxChunks[sfxIndex]) {
        Serial.printf("Playing SFX %d: %s\n", sfxIndex, sfxFilenames[sfxIndex]);
        int channel = Mix_PlayChannel(-1, sfxChunks[sfxIndex], 0);
        if (channel != -1) {
            // Set volume based on master volume
            float vol = constrain(masterVolume / 10.0f, 0.0f, 1.0f);
            Mix_Volume(channel, static_cast<int>(vol * MIX_MAX_VOLUME));
            activeChannels.push_back(channel);
            Serial.printf("SFX playing on channel %d\n", channel);
            return true;
        } else {
            Serial.printf("Mix_PlayChannel failed: %s\n", Mix_GetError());
        }
    } else {
        Serial.printf("SFX chunk %d is null\n", sfxIndex);
    }
    return false;
}

bool playRawSFX3D(int sfxIndex, float soundX, float soundY) {
    // Calculate distance from player to sound source
    float dx = soundX - playerX;
    float dy = soundY - playerY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // If sound is too far away, don't play it
    if (distance > MAX_AUDIO_DISTANCE) {
        return false;
    }
    
    // Calculate volume based on distance (linear falloff)
    float volume = 1.0f;
    if (distance > 0) {
        volume = 1.0f - (distance / MAX_AUDIO_DISTANCE);
        volume = constrain(volume, MIN_AUDIO_VOLUME, 1.0f);
    }
    
    ambientNoiseLevel++;
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX) return false;
    
    // Use SDL2 mixer to play the sound with 3D positioning
    if (sfxChunks[sfxIndex]) {
        int channel = Mix_PlayChannel(-1, sfxChunks[sfxIndex], 0);
        if (channel != -1) {
            // Apply distance-based volume and master volume
            float masterVol = masterVolume / 10.0f;
            float finalVolume = (sfxIndex == 23 ? volume / 2.0f : volume) * masterVol;
            Mix_Volume(channel, static_cast<int>(finalVolume * MIX_MAX_VOLUME));
            activeChannels.push_back(channel);
            return true;
        }
    }
    return false;
}

void serviceRawSFX() {
    // SDL2 handles this automatically, but we can clean up finished channels
    activeChannels.erase(
        std::remove_if(activeChannels.begin(), activeChannels.end(),
            [](int channel) { return Mix_Playing(channel) == 0; }),
        activeChannels.end()
    );
}

void freeSFX() {
    // Free SDL2 chunks
    for (int i = 0; i < NUM_SFX; i++) {
        if (sfxChunks[i]) {
            Mix_FreeChunk(sfxChunks[i]);
            sfxChunks[i] = nullptr;
        }
        if (sfxData[i]) {
            free(sfxData[i]);
            sfxData[i] = nullptr;
        }
    }
    
    // Stop all channels
    for (int channel : activeChannels) {
        Mix_HaltChannel(channel);
    }
    activeChannels.clear();
}

bool loadSFXtoRAM() {
    for (int i = 0; i < NUM_SFX; i++) {
        std::string filename = std::string("./Audio/") + sfxFilenames[i];
        
        // Load as SDL chunk for immediate playback
        sfxChunks[i] = Mix_LoadWAV(filename.c_str());
        if (!sfxChunks[i]) {
            Serial.printf("Failed to load SFX as SDL chunk: %s - Error: %s\n", filename.c_str(), Mix_GetError());
            continue;
        }
        
        Serial.printf("Loaded SFX: %s (%zu bytes)\n", filename.c_str(), sfxChunks[i]->alen);
    }
    return true;
}