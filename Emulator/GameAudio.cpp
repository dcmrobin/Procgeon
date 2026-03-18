#include "Common.h"
#include "GameAudio.h"
#include "Player.h"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// GameAudio.cpp  — SDL2 audio backend
// Logic identical to original; only includes have changed.
// ─────────────────────────────────────────────────────────────────────────────

AudioPlayQueue      queue[MAX_SIMULTANEOUS_SFX];
AudioMixer4         mixer1;
AudioMixer4         mixer2;
AudioMixer4         musicMixer;
AudioMixer4         wavMixer;
AudioOutputI2S      audioOutput;
AudioControlSGTL5000 sgtl5000_1;

AudioConnection patchCord1 (queue[0], 0, mixer1, 0);
AudioConnection patchCord2 (queue[1], 0, mixer1, 1);
AudioConnection patchCord3 (queue[2], 0, mixer1, 2);
AudioConnection patchCord4 (queue[3], 0, mixer1, 3);
AudioConnection patchCord5 (queue[4], 0, mixer2, 0);
AudioConnection patchCord6 (queue[5], 0, mixer2, 1);
AudioConnection patchCord7 (queue[6], 0, mixer2, 2);
AudioConnection patchCord8 (queue[7], 0, mixer2, 3);
AudioConnection patchCord9 (mixer1,   0, musicMixer, 0);
AudioConnection patchCord10(mixer2,   0, musicMixer, 1);
// playWav objects now routed through wavMixer → musicMixer
AudioConnection patchCord11(playWav1, 0, wavMixer, 0);
AudioConnection patchCord12(playWav2, 0, wavMixer, 1);
AudioConnection patchCord13(playWav3, 0, wavMixer, 2);
AudioConnection patchCord14(wavMixer,    0, musicMixer, 2);
AudioConnection patchCord15(musicMixer,  0, audioOutput, 0);
AudioConnection patchCord16(musicMixer,  0, audioOutput, 1);

int   ambientNoiseLevel = 0;
int   masterVolume      = MASTER_VOLUME_DEFAULT;
float jukeboxVolume     = 0.0f;
float shopVolume     = 0.0f;

uint8_t*   sfxData[NUM_SFX]   = { nullptr };
size_t     sfxLength[NUM_SFX] = { 0 };
Mix_Chunk* sfxChunks[NUM_SFX] = { nullptr };

std::vector<int> activeChannels;

const char* sfxFilenames[NUM_SFX] = {
    "player_hurt.wav",         //  0
    "player_shoot.wav",        //  1
    "player_use.wav",          //  2
    "player_pickup.wav",       //  3
    "player_footstep.wav",     //  4
    "player_eat.wav",          //  5
    "player_drink.wav",        //  6
    "menu_select.wav",         //  7
    "menu_scroll.wav",         //  8
    "menu_pause.wav",          //  9
    "menu_gameOver.wav",       // 10
    "level_end.wav",           // 11
    "inventory_open.wav",      // 12
    "inventory_close.wav",     // 13
    "enemy_teleport.wav",      // 14
    "damsel_putDown.wav",      // 15
    "damsel_passive.wav",      // 16
    "damsel_hurt.wav",         // 17
    "damsel_good.wav",         // 18
    "damsel_footstep.wav",     // 19
    "damsel_carry.wav",        // 20
    "damsel_annoying.wav",     // 21
    "bullet_impactWall.wav",   // 22
    "bullet_impactEnemy.wav",  // 23
    "succubus_hey.wav",        // 24
};

RawSFXPlayback activeSFX[MAX_SIMULTANEOUS_SFX];

void initAudio() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        Serial.printf("SDL audio init failed: %s\n", SDL_GetError());
        return;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        Serial.printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return;
    }
    Mix_AllocateChannels(16);
    float vol = constrain(masterVolume / 10.0f, 0.0f, 1.0f);
    Mix_Volume(-1, static_cast<int>(vol * MIX_MAX_VOLUME));
    if (!loadSFXtoRAM())
        Serial.println("Failed to load SFX");
    else
        Serial.println("SFX loaded successfully");
    Serial.println("Audio initialized with SDL2");
}

void setJukeboxVolume(float v) {
    jukeboxVolume = constrain(v, 0.0f, 0.23f);
    playWav2.volume(jukeboxVolume * (masterVolume / 10.0f));
}

void setShopVolume(float v) {
    shopVolume = constrain(v, 0.0f, 0.23f);
    playWav3.volume(shopVolume * (masterVolume / 10.0f));
}

bool playRawSFX(int sfxIndex) {
    ambientNoiseLevel++;
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX) return false;
    if (!sfxChunks[sfxIndex]) return false;
    int channel = Mix_PlayChannel(-1, sfxChunks[sfxIndex], 0);
    if (channel == -1) return false;
    float vol = constrain(masterVolume / 10.0f, 0.0f, 1.0f);
    Mix_Volume(channel, static_cast<int>(vol * MIX_MAX_VOLUME));
    activeChannels.push_back(channel);
    return true;
}

bool playRawSFX3D(int sfxIndex, float soundX, float soundY) {
    float dx   = soundX - playerX;
    float dy   = soundY - playerY;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist > MAX_AUDIO_DISTANCE) return false;

    float volume = (dist > 0.0f)
        ? constrain(1.0f - dist / MAX_AUDIO_DISTANCE, MIN_AUDIO_VOLUME, 1.0f)
        : 1.0f;

    ambientNoiseLevel++;
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX || !sfxChunks[sfxIndex]) return false;

    int channel = Mix_PlayChannel(-1, sfxChunks[sfxIndex], 0);
    if (channel == -1) return false;

    float masterVol = masterVolume / 10.0f;
    float finalVol  = (sfxIndex == 23 ? volume / 2.0f : volume) * masterVol;
    Mix_Volume(channel, static_cast<int>(finalVol * MIX_MAX_VOLUME));
    activeChannels.push_back(channel);
    return true;
}

void serviceRawSFX() {
    activeChannels.erase(
        std::remove_if(activeChannels.begin(), activeChannels.end(),
            [](int ch) { return Mix_Playing(ch) == 0; }),
        activeChannels.end());
}

void freeSFX() {
    for (int i = 0; i < NUM_SFX; i++) {
        if (sfxChunks[i]) { Mix_FreeChunk(sfxChunks[i]); sfxChunks[i] = nullptr; }
        if (sfxData[i])   { free(sfxData[i]);             sfxData[i]   = nullptr; }
    }
    for (int ch : activeChannels) Mix_HaltChannel(ch);
    activeChannels.clear();
}

bool loadSFXtoRAM() {
    for (int i = 0; i < NUM_SFX; i++) {
        std::string path = std::string("./Audio/") + sfxFilenames[i];
        sfxChunks[i] = Mix_LoadWAV(path.c_str());
        if (!sfxChunks[i])
            Serial.printf("Failed to load SFX: %s - %s\n", path.c_str(), Mix_GetError());
        else
            Serial.printf("Loaded SFX: %s\n", path.c_str());
    }
    return true;
}