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
AudioPlaySdWav      playWav1;
AudioPlaySdWav      playWav2;
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

// Initialize the audio system with SDL2
void initAudio() {
    // SDL2 audio is initialized in main.cpp, just set up volumes
    float vol = constrain(masterVolume / 10.0f, 0.0f, 1.0f);
    sgtl5000_1.volume(vol);
    
    // Pre-load all sound effects
    loadSFXtoRAM();
    
    //Serial.println("Audio initialized with SDL2");
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
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX) return false;
    
    // Use SDL2 mixer to play the sound
    if (sfxChunks[sfxIndex]) {
        int channel = Mix_PlayChannel(-1, sfxChunks[sfxIndex], 0);
        if (channel != -1) {
            // Set volume based on master volume
            float vol = constrain(masterVolume / 10.0f, 0.0f, 1.0f);
            Mix_Volume(channel, static_cast<int>(vol * MIX_MAX_VOLUME));
            activeChannels.push_back(channel);
            return true;
        }
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
        std::string filename = std::string("Audio/") + sfxFilenames[i];
        
        // Try to load as WAV first (SDL2 preferred format)
        std::string wavFilename = filename;
        size_t dotPos = wavFilename.find_last_of('.');
        if (dotPos != std::string::npos) {
            wavFilename = wavFilename.substr(0, dotPos) + ".wav";
        }
        
        sfxChunks[i] = Mix_LoadWAV(wavFilename.c_str());
        if (!sfxChunks[i]) {
            // Fall back to raw file loading if WAV not found
            SDClass::File f = SD.open(filename.c_str());
            if (!f) {
                //Serial.printf("Failed to open %s\n", filename.c_str());
                continue;
            }

            size_t len = f.size();
            if (len > MAX_SFX_SIZE) len = MAX_SFX_SIZE;

            sfxData[i] = (uint8_t*)malloc(len);
            if (!sfxData[i]) {
                //Serial.printf("Failed to allocate memory for %s\n", filename.c_str());
                f.close();
                continue;
            }

            f.read(sfxData[i], len);
            sfxLength[i] = len;
            f.close();
            
            // Convert raw to WAV chunk for SDL2
            // Note: You'll need to know the raw format (sample rate, channels, etc.)
            // This is a simplified version - you may need to adjust based on your raw format
            SDL_AudioSpec spec;
            spec.freq = 44100;
            spec.format = AUDIO_S16;
            spec.channels = 1;
            spec.silence = 0;
            spec.samples = 4096;
            spec.callback = nullptr;
            spec.userdata = nullptr;
            
            SDL_AudioCVT cvt;
            if (SDL_BuildAudioCVT(&cvt, AUDIO_S16, 1, 44100, spec.format, spec.channels, spec.freq) > 0) {
                cvt.len = len;
                cvt.buf = (Uint8*)malloc(cvt.len * cvt.len_mult);
                memcpy(cvt.buf, sfxData[i], len);
                if (SDL_ConvertAudio(&cvt) == 0) {
                    sfxChunks[i] = Mix_QuickLoad_RAW(cvt.buf, cvt.len_cvt);
                }
            }
        } else {
            //Serial.printf("Loaded WAV: %s\n", wavFilename.c_str());
        }
    }
    return true;
}